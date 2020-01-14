#include "unity.h"
#include "can.h"
#include "status.h"
#include "exported_enums.h"
#include "can_transmit.h"
#include "can_msg_defs.h"
#include "ms_test_helpers.h"
#include "log.h"
#include "event_queue.h"
#include "centre_console_events.h"
#include "power_on_sequence_fsm.h"
#include "interrupt.h"
#include "test_helpers.h"
#include "gpio.h"
#include <string.h>

typedef struct TestCanMessageAssertion {
  CanMessageId message_id;
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
} TestCanMessageAssertion;

static PowerOnSequenceFsmStorage s_sequence_storage;
static CanStorage s_can_storage;
static CanAckStatus s_ack_status;
static TestCanMessageAssertion s_can_message_assertion;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  *ack_reply = s_ack_status;
  s_can_message_assertion.message_id = msg->msg_id;
  s_can_message_assertion.data = msg->data;
  return STATUS_CODE_OK;
}

static void prv_can_init(SystemCanDevice device) {
  const CanSettings s_can_settings = {
    .device_id = device,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx = { .port = GPIO_PORT_A, .pin = 11 },
    .tx = { .port = GPIO_PORT_A, .pin = 12 },
    .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
  can_register_rx_default_handler(prv_rx_callback, NULL);
}

void prv_ack_msg_manually(SystemCanMessage msg_id, SystemCanDevice device_id) {
  CanMessage msg = {
    .type = CAN_MSG_TYPE_ACK,
    .msg_id = msg_id,
    .source_id = device_id
  };
  TEST_ASSERT_OK(can_ack_handle_msg(&s_can_storage.ack_requests, &msg));
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  memset(&s_sequence_storage, 0, sizeof(s_sequence_storage));
  memset(&s_can_message_assertion, 0, sizeof(s_can_message_assertion));
  prv_can_init(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE);
  power_on_sequence_fsm_init(&s_sequence_storage);
}

void teardown_test(void) {}

void prv_assert_acked_message(CanMessageId message_id, uint64_t data, SystemCanDevice acking_device) {
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(s_can_message_assertion.data, data);
  TEST_ASSERT_EQUAL(message_id, s_can_message_assertion.message_id);
  prv_ack_msg_manually(message_id, acking_device);
}

void prv_assert_acked_message_no_data(CanMessageId message_id, SystemCanDevice acking_device) {
  prv_assert_acked_message(message_id, 0, acking_device);
}

void prv_assert_power_distribution_acked_turn_on_message(EERearPowerDistributionOutput output) {
  uint64_t power_output = ((uint64_t) 1) << (uint64_t) output;
  uint64_t message_data = power_output << 16 | power_output;
  prv_assert_acked_message(SYSTEM_CAN_MESSAGE_REAR_POWER, message_data, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
}

void prv_assert_acked_relay_message(EERelayId relay_id, SystemCanDevice acking_device) {
  uint64_t relay_output = ((uint64_t) 1) << (uint64_t) relay_id;
  uint64_t message_data = relay_output << 8 | relay_output;
  prv_assert_acked_message(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, message_data, acking_device);
}

void test_sequence_happy_path(void) {
  s_ack_status = CAN_ACK_STATUS_OK;

  // none -> turn_on_driver_display_battery
  Event e = { .id = POWER_ON_SEQUENCE_EVENT_BEGIN, .data = 0 };
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);

  // turn on driver display
  MS_TEST_HELPER_CAN_TX(CENTRE_CONSOLE_EVENT_CAN_TX);

  // raise event for auto-transition to next stage
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_DRIVER_DISPLAY_IS_ON, 0);

  MS_TEST_HELPER_CAN_RX(CENTRE_CONSOLE_EVENT_CAN_RX);
  uint32_t power_output = 1 << EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY;
  uint32_t message_data = power_output << 16 | power_output;
  TEST_ASSERT_EQUAL(message_data, s_can_message_assertion.data);
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_FRONT_POWER, s_can_message_assertion.message_id);
  
  // Transition to next state.
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);

  // Turn on bms carrier.
  prv_assert_power_distribution_acked_turn_on_message(EE_REAR_POWER_DISTRIBUTION_OUTPUT_BMS_CARRIER);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_BATTERY_IS_ON, 0);

  // transition to: confirm aux voltage
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);

  // confirm aux voltage
  prv_assert_acked_message_no_data(SYSTEM_CAN_MESSAGE_GET_AUX_STATUS, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_AUX_VOLTAGE_OK, 0);

  // transition to: confirm battery health
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, BATTERY_HEARTBEAT_EVENT_HEALTH_CHECK_REQUEST, 0);

  // transition to: turn on motor controller interface
  e.id = POWER_ON_SEQUENCE_EVENT_BATTERY_HEALTH_OK;
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);

  // Turn on motor controller interface.
  prv_assert_power_distribution_acked_turn_on_message(EE_REAR_POWER_DISTRIBUTION_OUTPUT_MCI);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_MOTOR_CONTROLLER_INTERFACE_IS_ON, 0);

  // transition to: close battery relays
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);
  prv_assert_acked_relay_message(EE_RELAY_ID_BATTERY, SYSTEM_CAN_DEVICE_BMS_CARRIER);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_BATTERY_RELAY_CLOSED, 0);

  // transition to: confirm dc dc
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);
  prv_assert_acked_message_no_data(SYSTEM_CAN_MESSAGE_GET_DC_DC_STATUS, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_DC_DC_OK, 0);

  // transition to: precharge
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);
  prv_assert_acked_message_no_data(SYSTEM_CAN_MESSAGE_START_PRECHARGE, SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_NO_OP, 0);

  // transition to: close motor relay
  e.id = POWER_ON_SEQUENCE_EVENT_PRECHARGE_COMPLETE;
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);
  prv_assert_acked_relay_message(EE_RELAY_ID_MOTOR_CONTROLLER, SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_MOTOR_RELAY_CLOSED, 0);

  // confirm turn on
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_COMPLETE, 0);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_ON_SEQUENCE_EVENT_RESET, 0);

  // go back to none
  TEST_ASSERT_TRUE(power_on_sequence_fsm_process_event(&s_sequence_storage, &e));
}
