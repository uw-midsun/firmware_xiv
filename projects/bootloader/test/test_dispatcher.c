#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "dispatcher.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "string.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_DATA_GRAM_ID 2
#define TEST_CLIENT_SCRIPT_ID 0

static uint8_t s_board_id = 1;

static CanStorage s_test_can_storage;
static CanSettings s_test_can_settings = {
  .loopback = true,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_DATAGRAM_EVENT_RX,
  .tx_event = CAN_DATAGRAM_EVENT_TX,
  .fault_event = CAN_DATAGRAM_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
};

static CanDatagramSettings s_test_datagram_settings = {
  .tx_event = DATAGRAM_EVENT_TX,
  .rx_event = DATAGRAM_EVENT_RX,
  .repeat_event = DATAGRAM_EVENT_REPEAT,
  .error_event = DATAGRAM_EVENT_ERROR,
  .error_cb = NULL,
};

#define TEST_DATA_LEN 2048

uint8_t s_rx_data[DGRAM_MAX_DATA_SIZE];
static uint16_t s_rx_data_len;

void setup_test(void) {
  init_datagram_helper(&s_test_can_storage, &s_test_can_settings, s_board_id,
                       &s_test_datagram_settings);
}

void teardown_test(void) {}

StatusCode prv_dispatch_check_cb(uint8_t *data, uint16_t len, void *context) {
  LOG_DEBUG("Callback called!\n");
  bool *cb_called = context;
  *cb_called = true;

  memcpy(s_rx_data, data, len);
  s_rx_data_len = len;
  return STATUS_CODE_OK;
}

void test_dispatch_calls_cb(void) {
  // tests basic dispatch calls the callback when datagram is recieved
  // and tests dispatch's tx_cmpl_cb resumes datagram rx correctly
  TEST_ASSERT_OK(dispatcher_init(s_board_id));

  uint8_t s_tx_data[TEST_DATA_LEN] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                       'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };

  CanDatagramTxConfig tx_config = {
    .dgram_type = TEST_DATA_GRAM_ID,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = 0,
    .data = NULL,
    .tx_cmpl_cb = tx_cmpl_cb,
  };

  bool cb_called = false;
  TEST_ASSERT_OK(
      dispatcher_register_callback(TEST_DATA_GRAM_ID, prv_dispatch_check_cb, &cb_called));

  mock_tx_datagram(&tx_config);
  Event e = { 0 };
  DATAGRAM_PROCESS_ALL(e);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  TEST_ASSERT_TRUE_MESSAGE(cb_called, "Callback wasn't called");
  TEST_ASSERT_EQUAL(0, s_rx_data_len);
}

void test_datagram_completeness(void) {
  TEST_ASSERT_OK(dispatcher_init(s_board_id));

  uint8_t s_tx_data[TEST_DATA_LEN] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                       'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };

  CanDatagramTxConfig tx_config = {
    .dgram_type = TEST_DATA_GRAM_ID,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = TEST_DATA_LEN,
    .data = s_tx_data,
    .tx_cmpl_cb = tx_cmpl_cb,
  };

  bool cb_called = false;
  TEST_ASSERT_OK(
      dispatcher_register_callback(TEST_DATA_GRAM_ID, prv_dispatch_check_cb, &cb_called));

  mock_tx_datagram(&tx_config);
  Event e = { 0 };
  DATAGRAM_PROCESS_ALL(e);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  TEST_ASSERT_TRUE(cb_called);
  TEST_ASSERT_EQUAL(TEST_DATA_LEN, s_rx_data_len);
  for (int i = 0; i < s_rx_data_len; ++i) {
    TEST_ASSERT_EQUAL(s_tx_data[i], s_rx_data[i]);  // test the data from dispatcher is complete
  }
}

void test_noexistant_callback(void) {
  // send a datagram id with no associated callback
  TEST_ASSERT_OK(dispatcher_init(s_board_id));

  uint8_t s_tx_data[TEST_DATA_LEN] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                                       'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p' };

  CanDatagramTxConfig tx_config = {
    .dgram_type = TEST_DATA_GRAM_ID,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = TEST_DATA_LEN,
    .data = s_tx_data,
    .tx_cmpl_cb = tx_cmpl_cb,
  };
  mock_tx_datagram(&tx_config);
  Event e = { 0 };
  DATAGRAM_PROCESS_ALL(e);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
}

void test_register_invalid_id(void) {
  TEST_ASSERT_NOT_OK(dispatcher_register_callback(NUM_BOOTLOADER_DATAGRAMS, NULL, NULL));
}
