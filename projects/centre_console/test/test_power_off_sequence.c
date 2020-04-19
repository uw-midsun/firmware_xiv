#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "power_off_sequence.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;
static EERelayState s_battery_relay_state;
static EEDriveOutput s_drive_output;
static PowerOffSequenceStorage s_storage;

static StatusCode prv_rx_relay_state_callback(const CanMessage *msg, void *context,
                                              CanAckStatus *ack_reply) {
  uint16_t relay_mask;
  uint16_t relay_state;
  CAN_UNPACK_SET_RELAY_STATES(msg, &relay_mask, &relay_state);
  TEST_ASSERT_EQUAL(1 << EE_RELAY_ID_BATTERY, relay_mask);
  s_battery_relay_state = relay_state & (1 << EE_RELAY_ID_BATTERY);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  const CanSettings s_can_settings = { .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                       .loopback = true,
                                       .bitrate = CAN_HW_BITRATE_500KBPS,
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .tx = { .port = GPIO_PORT_A, .pin = 12 },
                                       .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
                                       .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
                                       .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                         prv_rx_relay_state_callback, NULL));
}

void teardown_test(void) {}

void prv_assert_set_discharge(Event *e) {
  // send discharge message
  MS_TEST_HELPER_CAN_TX(CENTRE_CONSOLE_EVENT_CAN_TX);
  Event event = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(event, POWER_OFF_SEQUENCE_EVENT_DISCHARGE_COMPLETED, 0);
  MS_TEST_HELPER_CAN_RX(CENTRE_CONSOLE_EVENT_CAN_RX);
  *e = event;
}

void prv_assert_turn_off_everything(Event *e) {
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE,
                                         SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,
                                         CAN_ACK_STATUS_OK);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE,
                                         SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,
                                         CAN_ACK_STATUS_OK);
  Event event = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(event, POWER_OFF_SEQUENCE_EVENT_TURNED_OFF_EVERYTHING, 0);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  *e = event;
}

void prv_assert_open_battery_relays(Event *e) {
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                         SYSTEM_CAN_DEVICE_BMS_CARRIER, CAN_ACK_STATUS_OK);
  Event event = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(event, POWER_OFF_SEQUENCE_EVENT_BATTERY_RELAYS_OPENED, 0);
  *e = event;
}

void test_happy_path_should_power_off_the_car(void) {
  TEST_ASSERT_OK(power_off_sequence_init(&s_storage));
  Event e = { .id = POWER_OFF_SEQUENCE_EVENT_BEGIN, .data = 0 };

  // none -> state discharge precharge
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  prv_assert_set_discharge(&e);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // set discharge -> turn off everything
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  prv_assert_turn_off_everything(&e);

  // Turn off everything -> open battery relays
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  prv_assert_open_battery_relays(&e);

  // open battery relays -> power off complete
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_OFF_SEQUENCE_EVENT_COMPLETE, 0);

  // power off complete -> state none
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
}

void test_fault_during_turn_off_everything_sequence(void) {
  TEST_ASSERT_OK(power_off_sequence_init(&s_storage));
  Event e = { .id = POWER_OFF_SEQUENCE_EVENT_BEGIN, .data = 0 };

  // none -> state discharge precharge
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  prv_assert_set_discharge(&e);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // set discharge -> turn off everything
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE,
                                         SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,
                                         CAN_ACK_STATUS_OK);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_POWER_OFF_SEQUENCE,
                                         SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,
                                         CAN_ACK_STATUS_INVALID);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_OFF_SEQUENCE_EVENT_FAULT,
                                   EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING);

  // turn off everything -> fault
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(
      e, CENTRE_CONSOLE_POWER_EVENT_FAULT,
      ((StateTransitionFault){ .state_machine = POWER_OFF_SEQUENCE_STATE_MACHINE,
                               .fault_reason = EE_POWER_OFF_SEQUENCE_TURN_OFF_EVERYTHING })
          .raw);

  // fault -> none
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_fault_during_set_relay_sequence(void) {
  TEST_ASSERT_OK(power_off_sequence_init(&s_storage));
  Event e = { .id = POWER_OFF_SEQUENCE_EVENT_BEGIN, .data = 0 };

  // none -> state discharge precharge
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  prv_assert_set_discharge(&e);

  // set discharge -> turn off everything
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  prv_assert_turn_off_everything(&e);

  // turn off everything -> open battery relays
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  for (uint8_t i = 0; i < NUM_RELAY_TX_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
    MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                           SYSTEM_CAN_DEVICE_BMS_CARRIER, CAN_ACK_STATUS_INVALID);
  }
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_OFF_SEQUENCE_EVENT_FAULT,
                                   EE_POWER_OFF_SEQUENCE_OPEN_BATTERY_RELAYS);

  // open battery relays -> fault
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(
      e, CENTRE_CONSOLE_POWER_EVENT_FAULT,
      ((StateTransitionFault){ .state_machine = POWER_OFF_SEQUENCE_STATE_MACHINE,
                               .fault_reason = EE_POWER_OFF_SEQUENCE_OPEN_BATTERY_RELAYS })
          .raw);

  // fault -> none
  TEST_ASSERT_TRUE(power_off_sequence_process_event(&s_storage, &e));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
