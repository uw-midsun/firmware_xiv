#include "heartbeat_rx.h"

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

#define TEST_HEARTBEAT_RX_CAN_DEVICE_ID 10

typedef enum {
  TEST_HEARTBEAT_RX_RX_CAN_RX = 10,
  TEST_HEARTBEAT_RX_RX_CAN_TX,
  TEST_HEARTBEAT_RX_RX_CAN_FAULT,
} TestCanEvent;

typedef struct TestHeartbeatRxHandlerCtx {
  SystemCanMessage expected_msg_id;
  bool ret_code;
  bool executed;
} TestHeartbeatRxHandlerCtx;

static HeartbeatRxHandlerStorage s_hb_storage;
static CanStorage s_can_storage;

// CanAckRequestCb
static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  (void)num_remaining;
  CanAckStatus *expected_status = context;
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT, msg_id);
  TEST_ASSERT_EQUAL(TEST_HEARTBEAT_RX_CAN_DEVICE_ID, device);
  TEST_ASSERT_EQUAL(*expected_status, status);
  return STATUS_CODE_OK;
}

// HeartbeatRxHandler
static bool prv_heartbeat_rx_handler(CanMessageId msg_id, void *context) {
  TestHeartbeatRxHandlerCtx *data = context;
  TEST_ASSERT_EQUAL(data->expected_msg_id, msg_id);
  data->executed = true;
  return data->ret_code;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_HEARTBEAT_RX_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = TEST_HEARTBEAT_RX_RX_CAN_RX,
    .tx_event = TEST_HEARTBEAT_RX_RX_CAN_TX,
    .fault_event = TEST_HEARTBEAT_RX_RX_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
}

void teardown_test(void) {}

void test_heartbeat_rx(void) {
  TestHeartbeatRxHandlerCtx context = {
    .expected_msg_id = SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
    .ret_code = true,
    .executed = false,
  };

  HeartbeatRxHandlerStorage hb_storage = {};
  TEST_ASSERT_OK(heartbeat_rx_register_handler(&hb_storage, SYSTEM_CAN_MESSAGE_POWERTRAIN_HEARTBEAT,
                                               prv_heartbeat_rx_handler, (void *)&context));
  CanAckStatus expected_status = CAN_ACK_STATUS_OK;
  CanAckRequest req = {
    .callback = prv_ack_callback,
    .context = &expected_status,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_HEARTBEAT_RX_CAN_DEVICE_ID),
  };

  CAN_TRANSMIT_POWERTRAIN_HEARTBEAT(&req);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_HEARTBEAT_RX_RX_CAN_TX, TEST_HEARTBEAT_RX_RX_CAN_RX);
  TEST_ASSERT_TRUE(context.executed);

  expected_status = CAN_ACK_STATUS_INVALID;
  context.ret_code = false;
  CAN_TRANSMIT_POWERTRAIN_HEARTBEAT(&req);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_HEARTBEAT_RX_RX_CAN_TX, TEST_HEARTBEAT_RX_RX_CAN_RX);
  TEST_ASSERT_TRUE(context.executed);
}
