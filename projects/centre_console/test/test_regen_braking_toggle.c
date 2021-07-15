#include "regen_braking_toggle.h"

#include "can.h"
#include "can_ack.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage = { 0 };
static uint8_t s_times_callback_called;
static CanAckStatus s_ack_reply_value;

static StatusCode prv_regen_callback(const CanMessage *msg, void *context,
                                     CanAckStatus *ack_reply) {
  *ack_reply = s_ack_reply_value;
  s_times_callback_called++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  s_times_callback_called = 0;
  s_ack_reply_value = CAN_ACK_STATUS_OK;
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER,
                                  CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX,
                                  CENTRE_CONSOLE_EVENT_CAN_FAULT);
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_REGEN_BRAKING, prv_regen_callback, NULL));
  TEST_ASSERT_OK(regen_braking_toggle_init());
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
}

void teardown_test(void) {}

void test_toggle_with_ack(void) {
  CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST();
  // First process REGEN_BRAKING_TOGGLE_REQUEST message
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // Then process REGEN_BRAKING message with ACK
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(3, s_times_callback_called);

  CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(4, s_times_callback_called);
}

void test_toggle_ack_failed(void) {
  s_ack_reply_value = CAN_ACK_STATUS_INVALID;

  CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST();
  // First process REGEN_BRAKING_TOGGLE_REQUEST message
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // Then process REGEN_BRAKING message with ACK
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // Then process resulting REGEN_BRAKING message to revert state
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(3, s_times_callback_called);

  CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(5, s_times_callback_called);

  CAN_TRANSMIT_REGEN_BRAKING_TOGGLE_REQUEST();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(7, s_times_callback_called);
}

void test_manual_toggle_with_ack(void) {
  s_ack_reply_value = CAN_ACK_STATUS_OK;
  TEST_ASSERT_OK(set_regen_braking_state(true));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  TEST_ASSERT_OK(set_regen_braking_state(false));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(3, s_times_callback_called);

  TEST_ASSERT_OK(set_regen_braking_state(false));
  // No message sent since setting to same state
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(3, s_times_callback_called);

  TEST_ASSERT_OK(set_regen_braking_state(true));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(4, s_times_callback_called);
}

void test_manual_toggle_ack_failed(void) {
  s_ack_reply_value = CAN_ACK_STATUS_INVALID;

  TEST_ASSERT_OK(set_regen_braking_state(true));
  // Then process REGEN_BRAKING message with ACK
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // Then process resulting REGEN_BRAKING message to revert state
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // No change to state
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(3, s_times_callback_called);

  TEST_ASSERT_OK(set_regen_braking_state(false));
  // No messages sent since setting to same state
  TEST_ASSERT_FALSE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(3, s_times_callback_called);
}

void test_event_trigger(void) {
  s_ack_reply_value = CAN_ACK_STATUS_OK;
  // False by default
  TEST_ASSERT_FALSE(get_regen_braking_state());
  Event power_sequence_event = { .id = POWER_MAIN_SEQUENCE_EVENT_COMPLETE, .data = 0 };
  // State should be true after POWER_MAIN_SEQUENCE_EVENT_COMPLETE event is passed
  TEST_ASSERT_TRUE(regen_braking_process_event(&power_sequence_event));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(get_regen_braking_state());
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}
