#include "can_rx.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_RX_NUM_HANDLERS 10

static CanRxHandlers s_rx_handlers;
static CanRxHandler s_rx_handler_storage[TEST_CAN_RX_NUM_HANDLERS];

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  *ack_reply = (CanAckStatus)context;

  return STATUS_CODE_OK;
}

void setup_test(void) {
  can_rx_init(&s_rx_handlers, s_rx_handler_storage, TEST_CAN_RX_NUM_HANDLERS);
}

void teardown_test(void) {}

void test_can_rx_handle(void) {
  StatusCode ret;
  ret = can_rx_register_handler(&s_rx_handlers, 0x7FF, prv_rx_callback, (void *)0x00);
  TEST_ASSERT_OK(ret);
  ret = can_rx_register_handler(&s_rx_handlers, 0x08, prv_rx_callback, (void *)0x01);
  TEST_ASSERT_OK(ret);
  ret = can_rx_register_handler(&s_rx_handlers, 0x01, prv_rx_callback, (void *)0x02);
  TEST_ASSERT_OK(ret);
  ret = can_rx_register_handler(&s_rx_handlers, 0x02, prv_rx_callback, (void *)0x03);
  TEST_ASSERT_OK(ret);

  CanRxHandler *handler = NULL;
  handler = can_rx_get_handler(&s_rx_handlers, 0x08);
  TEST_ASSERT_NOT_NULL(handler);
  TEST_ASSERT_EQUAL(0x01, handler->context);

  handler = can_rx_get_handler(&s_rx_handlers, 0x01);
  TEST_ASSERT_NOT_NULL(handler);
  TEST_ASSERT_EQUAL(0x02, handler->context);

  handler = can_rx_get_handler(&s_rx_handlers, 0x00);
  TEST_ASSERT_NULL(handler);

  handler = can_rx_get_handler(&s_rx_handlers, 0x08);
  TEST_ASSERT_NOT_NULL(handler);
  TEST_ASSERT_EQUAL(0x01, handler->context);
}

void test_can_rx_duplicate(void) {
  StatusCode ret;
  ret = can_rx_register_handler(&s_rx_handlers, 0x01, prv_rx_callback, (void *)0x00);
  TEST_ASSERT_OK(ret);
  ret = can_rx_register_handler(&s_rx_handlers, 0x01, prv_rx_callback, (void *)0x01);
  TEST_ASSERT_NOT_EQUAL(STATUS_CODE_OK, ret);

  CanRxHandler *handler = NULL;
  handler = can_rx_get_handler(&s_rx_handlers, 0x01);
  TEST_ASSERT_NOT_NULL(handler);
  TEST_ASSERT_EQUAL(0x00, handler->context);
}

void test_can_rx_default(void) {
  StatusCode ret;
  ret = can_rx_register_handler(&s_rx_handlers, 0x01, prv_rx_callback, (void *)0x00);
  TEST_ASSERT_OK(ret);

  ret = can_rx_register_default_handler(&s_rx_handlers, prv_rx_callback, (void *)0xA);
  TEST_ASSERT_OK(ret);

  CanRxHandler *handler = NULL;
  handler = can_rx_get_handler(&s_rx_handlers, 0x01);
  TEST_ASSERT_NOT_NULL(handler);
  TEST_ASSERT_EQUAL(0x00, handler->context);

  handler = can_rx_get_handler(&s_rx_handlers, 0x3);
  TEST_ASSERT_NOT_NULL(handler);
  TEST_ASSERT_EQUAL(0xA, handler->context);
}
