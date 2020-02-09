#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_tx_retry_wrapper.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "delay.h"
#include "drive_fsm.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define NUM_RETRIES 3

static CanTxRetryWrapperStorage s_storage = { 0 };
static CanStorage s_can_storage;
static CanAckStatus s_ack_status;
static StatusCode s_ack_cb_status;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("ACK: %d\n", s_ack_status);
  *ack_reply = s_ack_status;
  return s_ack_cb_status;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  CanTxRetryWrapperSettings settings = { .retries = NUM_RETRIES };
  const CanSettings s_can_settings = { .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                       .loopback = true,
                                       .bitrate = CAN_HW_BITRATE_500KBPS,
                                       .rx = { .port = GPIO_PORT_A, .pin = 11 },
                                       .tx = { .port = GPIO_PORT_A, .pin = 12 },
                                       .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
                                       .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
                                       .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_rx_callback, NULL));
  can_tx_retry_wrapper_init(&s_storage, &settings);
}

static void prv_ack_handle(SystemCanDevice acking_device, CanAckStatus status) {
  CanMessage msg = { .type = CAN_MSG_TYPE_ACK,
                     .msg_id = SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                     .source_id = acking_device,
                     .data = status };
  TEST_ASSERT_OK(can_ack_handle_msg(&s_can_storage.ack_requests, &msg));
}

static void prv_ack_pass(SystemCanDevice acking_device) {
  prv_ack_handle(acking_device, CAN_ACK_STATUS_OK);
}

static void prv_ack_fail(SystemCanDevice acking_device) {
  prv_ack_handle(acking_device, CAN_ACK_STATUS_UNKNOWN);
}

void teardown_test(void) {}

static void prv_can_tx(CanAckRequest *ack_ptr, void *context) {
  TEST_ASSERT_EQUAL(&s_storage, (CanTxRetryWrapperStorage *)context);
  uint8_t relay_mask = 1 << EE_RELAY_ID_MOTOR_CONTROLLER;
  uint8_t relay_state = relay_mask;
  CAN_TRANSMIT_SET_RELAY_STATES(ack_ptr, relay_state, relay_state);
}

void test_can_retry_emits_success_event(void) {
  s_ack_status = CAN_ACK_STATUS_OK;
  s_ack_cb_status = STATUS_CODE_OK;
  DriveFsmFault fault = { .fault_reason = DRIVE_FSM_FAULT_REASON_MCI_RELAY_STATE,
                          .fault_state = EE_RELAY_STATE_CLOSE };
  SystemCanDevice acking_device = SYSTEM_CAN_DEVICE_BMS_CARRIER;

  CanTxRetryWrapperRequest retry_request = {
    .completion_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE,
    .completion_event_data = DRIVE_STATE_REVERSE,
    .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
    .fault_event_data = fault.raw,
    .ack_bitset = CAN_ACK_EXPECTED_DEVICES(acking_device),
    .tx_callback = prv_can_tx,
    .tx_callback_context = &s_storage
  };

  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  prv_ack_pass(acking_device);

  Event e = { 0 };

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE,
                                   DRIVE_STATE_REVERSE);
}

void test_can_retry_emits_failure_event(void) {
  s_ack_status = CAN_ACK_STATUS_UNKNOWN;
  s_ack_cb_status = STATUS_CODE_INTERNAL_ERROR;

  DriveFsmFault fault = { .fault_reason = DRIVE_FSM_FAULT_REASON_MCI_RELAY_STATE,
                          .fault_state = EE_RELAY_STATE_CLOSE };

  SystemCanDevice acking_device = SYSTEM_CAN_DEVICE_BMS_CARRIER;

  CanTxRetryWrapperRequest retry_request = {
    .completion_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE,
    .completion_event_data = DRIVE_STATE_REVERSE,
    .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
    .fault_event_data = fault.raw,
    .ack_bitset = CAN_ACK_EXPECTED_DEVICES(acking_device),
    .tx_callback = prv_can_tx,
    .tx_callback_context = &s_storage
  };

  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));
  for (uint8_t i = 0; i < NUM_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
    prv_ack_fail(acking_device);
  }

  Event e = { 0 };

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_FAULT, fault.raw);
}

void test_can_retry_retries_indefinitely(void) {
  s_ack_status = CAN_ACK_STATUS_UNKNOWN;
  s_ack_cb_status = STATUS_CODE_INTERNAL_ERROR;

  DriveFsmFault fault = { .fault_reason = DRIVE_FSM_FAULT_REASON_MCI_RELAY_STATE,
                          .fault_state = EE_RELAY_STATE_CLOSE };

  SystemCanDevice acking_device = SYSTEM_CAN_DEVICE_BMS_CARRIER;

  CanTxRetryWrapperRequest retry_request = {
    .completion_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE,
    .completion_event_data = DRIVE_STATE_REVERSE,
    .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
    .fault_event_data = fault.raw,
    .retry_indefinitely = true,
    .ack_bitset = CAN_ACK_EXPECTED_DEVICES(acking_device),
    .tx_callback = prv_can_tx,
    .tx_callback_context = &s_storage
  };

  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));

  for (uint8_t i = 0; i < NUM_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
    prv_ack_fail(acking_device);
  }

  Event e = { 0 };

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_FAULT, fault.raw);

  for (uint8_t i = 0; i < NUM_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
    prv_ack_fail(acking_device);
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_FAULT, fault.raw);
  }
  // eventually fault clears
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  prv_ack_pass(acking_device);

  // success event gets raised
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE,
                                   DRIVE_STATE_REVERSE);

  // no further tx is attempted
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // counter is set back to zero

  // starting again
  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  prv_ack_fail(acking_device);

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
}
