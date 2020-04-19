#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "power_aux_sequence.h"
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

static PowerAuxSequenceFsmStorage s_sequence_storage;
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
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE, prv_rx_callback, NULL));
}

void prv_ack_msg_manually(SystemCanMessage msg_id, uint16_t ack_bitset) {
  while (ack_bitset) {
    uint8_t i = __builtin_ffs(ack_bitset) - 1;
    CanMessage msg = { .type = CAN_MSG_TYPE_ACK, .msg_id = msg_id, .source_id = i };
    ack_bitset &= ~(1 << i);
    TEST_ASSERT_OK(can_ack_handle_msg(&s_can_storage.ack_requests, &msg));
  }
}

static uint32_t s_ack_devices_lookup[NUM_EE_POWER_AUX_SEQUENCES] = { 0 };

static void prv_init_ack_lookup(void) {
  s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = CAN_ACK_EXPECTED_DEVICES(
      SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  memset(&s_sequence_storage, 0, sizeof(s_sequence_storage));
  memset(&s_can_message_assertion, 0, sizeof(s_can_message_assertion));
  prv_can_init(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE);
  power_aux_sequence_init(&s_sequence_storage);
  prv_init_ack_lookup();
}

void teardown_test(void) {}

void prv_assert_acked_message(CanMessageId message_id, uint64_t data, uint16_t ack_bitset) {
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(data, s_can_message_assertion.data);
  TEST_ASSERT_EQUAL(message_id, s_can_message_assertion.message_id);
  prv_ack_msg_manually(message_id, ack_bitset);
}

static EventId s_next_event_lookup[NUM_EE_POWER_AUX_SEQUENCES] = {
  [EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] = POWER_AUX_SEQUENCE_EVENT_AUX_STATUS_OK,
  [EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = POWER_AUX_SEQUENCE_EVENT_TURNED_ON_EVERYTHING,
};

void prv_assert_sequence_advance(EEPowerAuxSequence sequence) {
  Event e = { 0 };
  prv_assert_acked_message(SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE, sequence,
                           s_ack_devices_lookup[sequence]);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, s_next_event_lookup[sequence], 0);
  TEST_ASSERT_TRUE(power_aux_sequence_process_event(&s_sequence_storage, &e));
}

void test_sequence_happy_path(void) {
  Event e = { .id = POWER_AUX_SEQUENCE_EVENT_BEGIN, .data = 0 };
  power_aux_sequence_process_event(&s_sequence_storage, &e);

  EEPowerAuxSequence seq;
  for (seq = EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS; seq < NUM_EE_POWER_AUX_SEQUENCES; seq++) {
    LOG_DEBUG("seq: %d\n", seq);
    prv_assert_sequence_advance(seq);
  }

  // go back to state_none
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, POWER_AUX_SEQUENCE_EVENT_COMPLETE, 0);
  TEST_ASSERT_TRUE(power_aux_sequence_process_event(&s_sequence_storage, &e));
}

void test_sequence_fault_cancels_monitor(void) {
  Event e = { .id = POWER_AUX_SEQUENCE_EVENT_FAULT,
              .data = EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING };
  power_aux_sequence_process_event(&s_sequence_storage, &e);

  // must broadcast the fault to power_fsm
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(
      e, CENTRE_CONSOLE_POWER_EVENT_FAULT,
      ((StateTransitionFault){ .state_machine = POWER_AUX_SEQUENCE_STATE_MACHINE,
                               .fault_reason = EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING })
          .raw);
  // go back to none
  TEST_ASSERT_TRUE(power_aux_sequence_process_event(&s_sequence_storage, &e));
}
