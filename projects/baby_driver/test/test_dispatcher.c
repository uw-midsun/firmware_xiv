#include "dispatcher.h"

#include <string.h>

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_BABYDRIVER_MESSAGE_ID (NUM_BABYDRIVER_MESSAGES - 1)

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;

static uint8_t s_times_callback_called;
static uint8_t s_received_data[8];
static void *s_received_context;

static bool s_should_tx_result;
static StatusCode s_status_return;

static StatusCode prv_dispatcher_callback(uint8_t data[8], void *context, bool *tx_result) {
  s_times_callback_called++;
  memcpy(s_received_data, data, 8);
  s_received_context = context;
  *tx_result = s_should_tx_result;
  return s_status_return;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BABYDRIVER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(dispatcher_init());

  s_times_callback_called = 0;
  memset(s_received_data, 0, sizeof(s_received_data));
  s_received_context = NULL;
  s_should_tx_result = false;
  s_status_return = STATUS_CODE_OK;
}
void teardown_test(void) {}

// Test that we can successfully dispatch.
void test_dispatching_no_tx_result(void) {
  uint8_t arbitrary_context;  // value unused, but serves as test context for the callback
  TEST_ASSERT_OK(dispatcher_register_callback(TEST_BABYDRIVER_MESSAGE_ID, prv_dispatcher_callback,
                                              &arbitrary_context));
  TEST_ASSERT_EQUAL(0, s_times_callback_called);

  // send CAN message with our message ID - should receive same data
  uint8_t data[7] = { 1, 2, 3, 4, 5, 6, 7 };
  CAN_TRANSMIT_BABYDRIVER(TEST_BABYDRIVER_MESSAGE_ID, data[0], data[1], data[2], data[3], data[4],
                          data[5], data[6]);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(TEST_BABYDRIVER_MESSAGE_ID, s_received_data[0]);
  TEST_ASSERT_EQUAL(&arbitrary_context, s_received_context);

  // use a pointer to the second element to compare |data| with the non-ID part of |s_received_data|
  uint8_t *received_data = &s_received_data[1];
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, received_data, 7);

  // should not tx result, so no CAN events
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that TXing the result works.
void test_dispatching_with_tx_result(void) {
  s_should_tx_result = true;

  // register the same callback for both the test message and the status message to intercept it
  // note that this works even if the test message is the status message
  TEST_ASSERT_OK(
      dispatcher_register_callback(TEST_BABYDRIVER_MESSAGE_ID, prv_dispatcher_callback, NULL));
  TEST_ASSERT_OK(
      dispatcher_register_callback(BABYDRIVER_MESSAGE_STATUS, prv_dispatcher_callback, NULL));

  CAN_TRANSMIT_BABYDRIVER(TEST_BABYDRIVER_MESSAGE_ID, 0, 0, 0, 0, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  s_should_tx_result = false;  // avoid TXing result of the status message
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  s_should_tx_result = true;

  // the second message we received should be a status message with the status in the second byte
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);

  // try once more with a non-OK status
  s_status_return = STATUS_CODE_RESOURCE_EXHAUSTED;

  CAN_TRANSMIT_BABYDRIVER(TEST_BABYDRIVER_MESSAGE_ID, 0, 0, 0, 0, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  s_should_tx_result = false;  // avoid TXing result of the status message
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  s_should_tx_result = true;

  TEST_ASSERT_EQUAL(4, s_times_callback_called);
  TEST_ASSERT_EQUAL(BABYDRIVER_MESSAGE_STATUS, s_received_data[0]);
  TEST_ASSERT_EQUAL(s_status_return, s_received_data[1]);
}

// Test that we don't segfault when there's no registered callback for the message.
void test_nonexistant_callback(void) {
  CAN_TRANSMIT_BABYDRIVER(TEST_BABYDRIVER_MESSAGE_ID, 0, 0, 0, 0, 0, 0, 0);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(0, s_times_callback_called);
}

// Test that we fail gracefully on trying to register a callback for an invalid message ID.
void test_registering_invalid_callback(void) {
  TEST_ASSERT_NOT_OK(
      dispatcher_register_callback(NUM_BABYDRIVER_MESSAGES, prv_dispatcher_callback, NULL));
}
