#include "relay_rx.h"

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_mcu.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_RELAY_CAN_DEVICE_ID 10

typedef enum {
  TEST_RELAY_RX_STATE_OPEN = 0,
  TEST_RELAY_RX_STATE_CLOSE,
  NUM_TEST_RELAY_RX_STATES,
} TestRelayStates;

typedef enum {
  TEST_RELAY_RX_CAN_RX = 10,
  TEST_RELAY_RX_CAN_TX,
  TEST_RELAY_RX_CAN_FAULT,
} TestCanEvent;

typedef struct TestRelayRxHandlerCtx {
  StatusCode ret_code;
  SystemCanMessage expected_msg_id;
  uint8_t expected_state;
  bool executed;
} TestRelayRxHandlerCtx;

static CanStorage s_can_storage;

// CanAckRequestCb
static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  (void)num_remaining;
  CanAckStatus *expected_status = context;
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN, msg_id);
  TEST_ASSERT_EQUAL(TEST_RELAY_CAN_DEVICE_ID, device);
  TEST_ASSERT_EQUAL(*expected_status, status);
  return STATUS_CODE_OK;
}

// RelayRxHandler
static StatusCode prv_relay_rx_handler(SystemCanMessage msg_id, uint8_t state, void *context) {
  TestRelayRxHandlerCtx *data = context;
  TEST_ASSERT_EQUAL(data->expected_msg_id, msg_id);
  TEST_ASSERT_EQUAL(data->expected_state, state);
  data->executed = true;
  return data->ret_code;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_RELAY_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_250KBPS,
    .rx_event = TEST_RELAY_RX_CAN_RX,
    .tx_event = TEST_RELAY_RX_CAN_TX,
    .fault_event = TEST_RELAY_RX_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
}

void teardown_test(void) {}

void test_relay_rx_guards(void) {
  TestRelayRxHandlerCtx context = {
    .ret_code = STATUS_CODE_OK,
    .expected_msg_id = SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN,
    .expected_state = TEST_RELAY_RX_STATE_OPEN,
    .executed = false,
  };
  RelayRxStorage relay_storage_a = { 0 };
  TEST_ASSERT_OK(relay_rx_configure_handler(&relay_storage_a, SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN,
                                            NUM_TEST_RELAY_RX_STATES, prv_relay_rx_handler,
                                            &context));
  // Fail to register duplicates.
  RelayRxStorage relay_storage_b = { 0 };
  TEST_ASSERT_EQUAL(
      STATUS_CODE_RESOURCE_EXHAUSTED,
      relay_rx_configure_handler(&relay_storage_b, SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN,
                                 NUM_TEST_RELAY_RX_STATES, prv_relay_rx_handler, &context));
}

void test_relay_rx(void) {
  TestRelayRxHandlerCtx context = {
    .ret_code = STATUS_CODE_OK,
    .expected_msg_id = SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN,
    .expected_state = TEST_RELAY_RX_STATE_OPEN,
    .executed = false,
  };
  RelayRxStorage relay_storage = { 0 };
  TEST_ASSERT_OK(relay_rx_configure_handler(&relay_storage, SYSTEM_CAN_MESSAGE_BATTERY_RELAY_MAIN,
                                            NUM_TEST_RELAY_RX_STATES, prv_relay_rx_handler,
                                            &context));

  CanAckStatus expected_status = CAN_ACK_STATUS_OK;
  CanAckRequest req = {
    .callback = prv_ack_callback,
    .context = &expected_status,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_RELAY_CAN_DEVICE_ID),
  };

  // Open.
  CAN_TRANSMIT_BATTERY_RELAY_MAIN(&req, TEST_RELAY_RX_STATE_OPEN);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_RELAY_RX_CAN_TX, TEST_RELAY_RX_CAN_RX);
  TEST_ASSERT_TRUE(context.executed);
  context.executed = false;

  // Close.
  context.expected_state = TEST_RELAY_RX_STATE_CLOSE;
  CAN_TRANSMIT_BATTERY_RELAY_MAIN(&req, TEST_RELAY_RX_STATE_CLOSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_RELAY_RX_CAN_TX, TEST_RELAY_RX_CAN_RX);
  TEST_ASSERT_TRUE(context.executed);
  context.executed = false;

  // Invalid.
  expected_status = CAN_ACK_STATUS_INVALID;
  CAN_TRANSMIT_BATTERY_RELAY_MAIN(&req, NUM_TEST_RELAY_RX_STATES);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_RELAY_RX_CAN_TX, TEST_RELAY_RX_CAN_RX);
  TEST_ASSERT_FALSE(context.executed);

  // Fail to update relay.
  context.ret_code = STATUS_CODE_INTERNAL_ERROR;
  CAN_TRANSMIT_BATTERY_RELAY_MAIN(&req, TEST_RELAY_RX_STATE_CLOSE);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_RELAY_RX_CAN_TX, TEST_RELAY_RX_CAN_RX);
  TEST_ASSERT_TRUE(context.executed);
}
