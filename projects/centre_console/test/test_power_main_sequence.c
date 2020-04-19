#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "power_main_sequence.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

typedef struct TestCanMessageAssertion {
  CanMessageId message_id;
  union {
    uint64_t data;
    uint32_t data_u32[2];
    uint16_t data_u16[4];
    uint8_t data_u8[8];
  };
} TestCanMessageAssertion;

static PowerMainSequenceFsmStorage s_sequence_storage;
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
  const CanSettings s_can_settings = { .device_id = device,
                                       .loopback = true,
                                       .bitrate = CAN_HW_BITRATE_500KBPS,
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .tx = { .port = GPIO_PORT_A, .pin = 12 },
                                       .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
                                       .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
                                       .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));

  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, prv_rx_callback, NULL));
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_rx_callback, NULL));
}

void prv_ack_msg_manually(SystemCanMessage msg_id, uint16_t ack_bitset) {
  while (ack_bitset) {
    uint8_t i = __builtin_ffs(ack_bitset) - 1;
    CanMessage msg = { .type = CAN_MSG_TYPE_ACK, .msg_id = msg_id, .source_id = i };
    ack_bitset &= ~(1 << i);
    TEST_ASSERT_OK(can_ack_handle_msg(&s_can_storage.ack_requests, &msg));
  }
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  memset(&s_sequence_storage, 0, sizeof(s_sequence_storage));
  memset(&s_can_message_assertion, 0, sizeof(s_can_message_assertion));
  prv_can_init(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE);
  power_main_sequence_init(&s_sequence_storage);
}

void teardown_test(void) {}

void prv_assert_acked_message(CanMessageId message_id, uint64_t data, uint16_t ack_bitset) {
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(data, s_can_message_assertion.data);
  TEST_ASSERT_EQUAL(message_id, s_can_message_assertion.message_id);
  prv_ack_msg_manually(message_id, ack_bitset);
}

static EventId s_next_event_lookup[NUM_EE_POWER_MAIN_SEQUENCES] = {
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS] = POWER_MAIN_SEQUENCE_EVENT_AUX_STATUS_OK,
  [EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS] = POWER_MAIN_SEQUENCE_EVENT_DRIVER_DISPLAY_BMS_ON,
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS] = POWER_MAIN_SEQUENCE_EVENT_BATTERY_STATUS_OK,
  [EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS] = POWER_MAIN_SEQUENCE_EVENT_BATTERY_RELAYS_CLOSED,
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC] = POWER_MAIN_SEQUENCE_EVENT_DC_DC_OK,
  [EE_POWER_MAIN_SEQUENCE_TURN_ON_EVERYTHING] = POWER_MAIN_SEQUENCE_EVENT_TURNED_ON_EVERYTHING,
};

void prv_assert_acked_relay_message(EERelayId relay_id, uint8_t sequence) {
  uint16_t relay_output = ((uint64_t)1) << (uint64_t)relay_id;
  uint16_t relay_mask = relay_output;
  CanMessage message = { 0 };
  CAN_PACK_SET_RELAY_STATES(&message, relay_output, relay_mask);
  uint32_t *ack_devices_lookup = test_get_ack_devices_lookup();
  prv_assert_acked_message(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, message.data,
                           ack_devices_lookup[sequence]);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(
      e, s_next_event_lookup[EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS], 0);
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));
}

void prv_assert_sequence_advance(EEPowerMainSequence sequence) {
  Event e = { 0 };
  uint32_t *ack_devices_lookup = test_get_ack_devices_lookup();
  prv_assert_acked_message(SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE, sequence,
                           ack_devices_lookup[sequence]);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, s_next_event_lookup[sequence], 0);
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));
}

void test_sequence_happy_path(void) {
  Event e = { .id = POWER_MAIN_SEQUENCE_EVENT_BEGIN, .data = 0 };
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));

  EEPowerMainSequence seq;
  for (seq = EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS;
       seq < EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS; seq++) {
    prv_assert_sequence_advance(seq);
  }

  prv_assert_acked_relay_message(EE_RELAY_ID_BATTERY, seq++);

  for (seq = EE_POWER_MAIN_SEQUENCE_CONFIRM_DCDC; seq < NUM_EE_POWER_MAIN_SEQUENCES; seq++) {
    prv_assert_sequence_advance(seq);
  }
}

void test_sequence_fault_raises_fault_event(void) {
  Event e = { .id = POWER_MAIN_SEQUENCE_EVENT_BEGIN, .data = 0 };
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));

  EEPowerMainSequence seq;
  for (seq = EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS;
       seq < EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS; seq++) {
    prv_assert_sequence_advance(seq);
  }

  // Assert can message sent to check batteries
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  e.id = POWER_MAIN_SEQUENCE_EVENT_FAULT;
  e.data = EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS;
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));

  // Must broadcast the fault to power_fsm
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(
      e, CENTRE_CONSOLE_POWER_EVENT_FAULT,
      ((StateTransitionFault){ .state_machine = POWER_MAIN_SEQUENCE_STATE_MACHINE,
                               .fault_reason = EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS })
          .raw);

  // transition to none
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));

  // no other event must be raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Must reset to state none (this will fail if it's not in none)
  e.id = POWER_MAIN_SEQUENCE_EVENT_BEGIN;
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));
}

static SystemCanDevice s_ack_device_lookup[] = {
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS] = SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,
  [EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS] = SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR,
  [EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS] = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  [EE_POWER_MAIN_SEQUENCE_CLOSE_BATTERY_RELAYS] = SYSTEM_CAN_DEVICE_BMS_CARRIER
};

void test_sequence_can_ack_failure_raises_fault_event(void) {
  EEPowerMainSequence faulting_sequence;
  for (faulting_sequence = EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS;
       faulting_sequence < EE_POWER_MAIN_SEQUENCE_CONFIRM_BATTERY_STATUS; faulting_sequence++) {
    faulting_sequence = EE_POWER_MAIN_SEQUENCE_TURN_ON_DRIVER_BMS;
    Event e = { .id = POWER_MAIN_SEQUENCE_EVENT_BEGIN, .data = 0 };
    TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));

    EEPowerMainSequence seq;
    for (seq = EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS; seq < faulting_sequence; seq++) {
      prv_assert_sequence_advance(seq);
    }

    // Assert can
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

    // Make it fault
    uint32_t *ack_device_lookup = test_get_ack_devices_lookup();
    MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE,
                                           s_ack_device_lookup[faulting_sequence],
                                           CAN_ACK_STATUS_UNKNOWN);
    // We now must have a fault generated
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_MAIN_SEQUENCE_EVENT_FAULT, faulting_sequence);

    // Transition to fault
    TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));

    // Must broadcast the fault to power_fsm
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(
        e, CENTRE_CONSOLE_POWER_EVENT_FAULT,
        ((StateTransitionFault){ .state_machine = POWER_MAIN_SEQUENCE_STATE_MACHINE,
                                 .fault_reason = faulting_sequence })
            .raw);

    // transition to none
    TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));

    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  }
  Event e = { 0 };
  // Must be at state none (this will fail if it's not in none)
  e.id = POWER_MAIN_SEQUENCE_EVENT_BEGIN;
  TEST_ASSERT_TRUE(power_main_sequence_fsm_process_event(&s_sequence_storage, &e));
}
