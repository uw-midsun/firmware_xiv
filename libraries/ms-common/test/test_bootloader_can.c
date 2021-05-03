#include "bootloader_can.h"

#include <string.h>

#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define CLIENT_SCRIPT_CONTROLLER_BOARD_ID 0

typedef enum {
  TEST_CAN_EVENT_RX = 0,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;
static uint8_t s_times_callback_called;
static uint8_t s_received_data[8];
static size_t s_received_len;
static void *s_received_context;
bool s_received_is_start_message;
uint16_t s_board_id;

static StatusCode prv_bootloader_rx_callback(const CanMessage *msg, void *context,
                                             CanAckStatus *ack_reply) {
  s_times_callback_called++;
  memcpy(s_received_data, msg->data_u8, 8);
  s_received_len = msg->dlc;
  s_received_context = context;
  if (msg->type == CAN_MSG_TYPE_ACK) {
    s_received_is_start_message = true;
  } else {
    s_received_is_start_message = false;
  }
  TEST_ASSERT_EQUAL(msg->source_id, SYSTEM_CAN_DEVICE_BOOTLOADER);
  TEST_ASSERT_EQUAL(msg->msg_id, CLIENT_SCRIPT_CONTROLLER_BOARD_ID);
  return STATUS_CODE_OK;
}

void setup_test(void) {
  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  s_times_callback_called = 0;
  memset(s_received_data, 0, sizeof(s_received_data));
  s_received_len = 0;
  s_received_context = NULL;
  s_received_is_start_message = false;
  s_board_id = 0;

  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BOOTLOADER, TEST_CAN_EVENT_TX,
                                  TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(bootloader_can_init(&s_can_storage, &can_settings));
  TEST_ASSERT_OK(bootloader_can_register_handler(prv_bootloader_rx_callback, NULL));
}

void teardown_test(void) {}

// Test that a bootloader CAN message with cliend board ID can be succesfully sent
// and the appropreate callback called when received
void test_bootloader_can_received(void) {
  uint8_t data[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  TEST_ASSERT_OK(bootloader_can_transmit(data, 8, true, CLIENT_SCRIPT_CONTROLLER_BOARD_ID));

  // process bootloader message
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, s_received_data, 8);
  TEST_ASSERT_EQUAL(8, s_received_len);
  TEST_ASSERT_EQUAL(true, s_received_is_start_message);
  TEST_ASSERT_EQUAL(NULL, s_received_context);
}

// Test that a bootloader CAN message sent with non-client board ID is not received
void test_bootloader_can_not_received(void) {
  uint8_t data[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  TEST_ASSERT_OK(bootloader_can_transmit(data, 8, false, 2));

  // process bootloader message, and callback should not be called
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(0, s_times_callback_called);
}
