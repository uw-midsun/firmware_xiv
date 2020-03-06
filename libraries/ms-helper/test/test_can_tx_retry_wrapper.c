#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_tx_retry_wrapper.h"
#include "delay.h"
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

static bool s_callback_called = false;
static uint32_t s_captured_callback_data = 0;
static uint32_t s_callback_data = 0x12345;

static void prv_success_callback(void *context) {
  s_callback_called = true;
  s_captured_callback_data = *((uint32_t *)context);
}

typedef enum {
  RETRY_WRAPPER_EVENT_CAN_TX = 0,
  RETRY_WRAPPER_EVENT_CAN_RX,
  RETRY_WRAPPER_EVENT_CAN_FAULT,
  RETRY_WRAPPER_EVENT_SUCCESS,
  RETRY_WRAPPER_EVENT_FAIL,
} RetryWrapperEvent;

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
                                       .rx_event = RETRY_WRAPPER_EVENT_CAN_RX,
                                       .tx_event = RETRY_WRAPPER_EVENT_CAN_TX,
                                       .fault_event = RETRY_WRAPPER_EVENT_CAN_FAULT };
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_rx_callback, NULL));
  can_tx_retry_wrapper_init(&s_storage, &settings);
  s_callback_called = false;
  s_captured_callback_data = 0;
}

void teardown_test(void) {}

static void prv_can_tx(CanAckRequest *ack_ptr, void *context) {
  TEST_ASSERT_EQUAL(&s_storage, (CanTxRetryWrapperStorage *)context);
  uint8_t relay_id = EE_RELAY_ID_MOTOR_CONTROLLER;
  uint8_t relay_mask = 1 << relay_id;
  uint8_t relay_state = relay_mask;
  LOG_DEBUG("transmitting relay: %d states: %d\n", relay_id, relay_state);
  CAN_TRANSMIT_SET_RELAY_STATES(ack_ptr, relay_state, relay_state);
}

void test_can_retry_emits_success_event(void) {
  s_ack_status = CAN_ACK_STATUS_OK;
  s_ack_cb_status = STATUS_CODE_OK;
  SystemCanDevice acking_device = SYSTEM_CAN_DEVICE_BMS_CARRIER;

  uint8_t success_data = 0x12;
  uint8_t fail_data = 0x34;

  CanTxRetryWrapperRequest retry_request = {
    .retry_request =
        {
            .completion_event_id = RETRY_WRAPPER_EVENT_SUCCESS,
            .completion_event_data = success_data,
            .fault_event_id = RETRY_WRAPPER_EVENT_CAN_FAULT,
            .fault_event_data = fail_data,
        },
    .ack_bitset = CAN_ACK_EXPECTED_DEVICES(acking_device),
    .tx_callback = prv_can_tx,
    .tx_callback_context = &s_storage
  };

  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(RETRY_WRAPPER_EVENT_CAN_TX, RETRY_WRAPPER_EVENT_CAN_RX);

  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                         acking_device, CAN_ACK_STATUS_OK);

  Event e = { 0 };

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, RETRY_WRAPPER_EVENT_SUCCESS, success_data);

  // no events should be remaining
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_can_retry_emits_failure_event(void) {
  s_ack_status = CAN_ACK_STATUS_UNKNOWN;
  s_ack_cb_status = STATUS_CODE_INTERNAL_ERROR;

  SystemCanDevice acking_device = SYSTEM_CAN_DEVICE_BMS_CARRIER;
  uint16_t fault_reason = 0x1234;
  uint16_t success_data = 0x5678;

  CanTxRetryWrapperRequest retry_request = { .retry_request =
                                                 {
                                                     .completion_event_id =
                                                         RETRY_WRAPPER_EVENT_SUCCESS,
                                                     .completion_event_data = success_data,
                                                     .fault_event_id = RETRY_WRAPPER_EVENT_FAIL,
                                                     .fault_event_data = fault_reason,
                                                 },
                                             .ack_bitset = CAN_ACK_EXPECTED_DEVICES(acking_device),
                                             .tx_callback = prv_can_tx,
                                             .tx_callback_context = &s_storage };

  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));
  for (uint8_t i = 0; i < NUM_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(RETRY_WRAPPER_EVENT_CAN_TX, RETRY_WRAPPER_EVENT_CAN_RX);
    MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                           acking_device, CAN_ACK_STATUS_INVALID);
  }

  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, RETRY_WRAPPER_EVENT_FAIL, fault_reason);

  // no further events must be raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // Retry counter should be reset to 0
  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(RETRY_WRAPPER_EVENT_CAN_TX, RETRY_WRAPPER_EVENT_CAN_RX);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                         acking_device, CAN_ACK_STATUS_INVALID);

  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_NOT_EQUAL(e.id, RETRY_WRAPPER_EVENT_FAIL);
}

void test_can_retry_retries_indefinitely(void) {
  s_ack_status = CAN_ACK_STATUS_UNKNOWN;
  s_ack_cb_status = STATUS_CODE_INTERNAL_ERROR;

  uint16_t fault_data = 0x1235;
  uint16_t success_data = 0x4569;
  SystemCanDevice acking_device = SYSTEM_CAN_DEVICE_BMS_CARRIER;

  CanTxRetryWrapperRequest retry_request = { .retry_request =
                                                 {
                                                     .completion_event_id =
                                                         RETRY_WRAPPER_EVENT_SUCCESS,
                                                     .completion_event_data = success_data,
                                                     .fault_event_id = RETRY_WRAPPER_EVENT_FAIL,
                                                     .fault_event_data = fault_data,
                                                     .retry_indefinitely = true,
                                                 },
                                             .ack_bitset = CAN_ACK_EXPECTED_DEVICES(acking_device),
                                             .tx_callback = prv_can_tx,
                                             .tx_callback_context = &s_storage };

  TEST_ASSERT_OK(can_tx_retry_wrapper_register_success_callback(&s_storage, prv_success_callback,
                                                                &s_callback_data));

  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));

  for (uint8_t i = 0; i < NUM_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(RETRY_WRAPPER_EVENT_CAN_TX, RETRY_WRAPPER_EVENT_CAN_RX);
    MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                           acking_device, CAN_ACK_STATUS_INVALID);
  }

  Event e = { 0 };

  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, RETRY_WRAPPER_EVENT_FAIL, fault_data);

  for (uint8_t i = 0; i < NUM_RETRIES; i++) {
    MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(RETRY_WRAPPER_EVENT_CAN_TX, RETRY_WRAPPER_EVENT_CAN_RX);
    MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                           acking_device, CAN_ACK_STATUS_INVALID);
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, RETRY_WRAPPER_EVENT_FAIL, fault_data);
  }
  // eventually fault clears
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(RETRY_WRAPPER_EVENT_CAN_TX, RETRY_WRAPPER_EVENT_CAN_RX);

  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                         acking_device, CAN_ACK_STATUS_OK);

  // success event gets raised
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, RETRY_WRAPPER_EVENT_SUCCESS, success_data);

  // callback also gets called
  TEST_ASSERT_EQUAL(s_captured_callback_data, s_callback_data);
  TEST_ASSERT_TRUE(s_callback_called);

  // no further tx is attempted
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // counter is set back to zero

  // starting again
  TEST_ASSERT_OK(can_tx_retry_send(&s_storage, &retry_request));

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(RETRY_WRAPPER_EVENT_CAN_TX, RETRY_WRAPPER_EVENT_CAN_RX);
  MS_TEST_HELPER_ACK_MESSAGE_WITH_STATUS(s_can_storage, SYSTEM_CAN_MESSAGE_SET_RELAY_STATES,
                                         acking_device, CAN_ACK_STATUS_INVALID);

  event_process(&e);
  TEST_ASSERT_NOT_EQUAL(e.id, RETRY_WRAPPER_EVENT_FAIL);
}
