#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "ping.h"
#include "query.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CLIENT_SCRIPT_ID 0

static uint8_t s_client_id = 0;
static uint8_t s_board_id = 2;

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

static uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE];
static uint8_t s_rx_data[DGRAM_MAX_DATA_SIZE];
static uint16_t s_rx_data_len;

// send a query and assert the response matches the expected responses
void prv_test_response(uint8_t *query, size_t query_len, uint8_t *expected, size_t expected_len) {
  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_QUERY_COMMAND,
    .destination_nodes_len = 1,
    .destination_nodes = &s_client_id,
    .data_len = query_len,
    .data = query,
    .tx_cmpl_cb = tx_cmpl_cb,
  };

  CanDatagramRxConfig rx_config = {
    .destination_nodes = s_destination_nodes,
    .data = s_rx_data,
    .node_id = 0,  // listen to all
    .rx_cmpl_cb = NULL,
  };

  dgram_helper_mock_tx_datagram(&tx_config);
  dgram_helper_mock_rx_datagram(&rx_config);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

  TEST_ASSERT_EQUAL(BOOTLOADER_DATAGRAM_QUERY_RESPONSE, rx_config.dgram_type);
  TEST_ASSERT_EQUAL(rx_config.data_len, expected_len);
  for (int i = 0; i < rx_config.data_len; ++i) {
    TEST_ASSERT_EQUAL(rx_config.data[i], expected[i]);
  }
}

// send a query and assert that no response is sent
void prv_test_no_response(uint8_t *query, size_t query_len) {
  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_QUERY_COMMAND,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = query_len,
    .data = query,
    .tx_cmpl_cb = tx_cmpl_cb,
  };
  CanDatagramRxConfig rx_config = {
    .destination_nodes = s_destination_nodes,
    .data = s_rx_data,
    .node_id = 0,  // listen to all
    .rx_cmpl_cb = NULL,
  };

  dgram_helper_mock_tx_datagram(&tx_config);
  dgram_helper_assert_no_response();
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  crc32_init();

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
  dispatcher_init(s_board_id);
}

void teardown_test(void) {}

void test_query(void) {
  // test max sized response (almost maxed, s_board_id is not the longest possible)
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910",
    .project_present = true,
    .project_name = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910",
    .project_info = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910",
    .git_version = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910",
  };
  query_init(&config);

  // "id" : [255]
  // "name" : ["abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910"]
  // "current_project" : ["abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910"]
  // "project_info" : ["abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910"]
  // "git_version" : ["abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910"]
  size_t len = 262;
  // exact query, so this is also the expected response
  uint8_t data[262] = {
    0x08, 0x02, 0x12, 0x3F, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C,
    0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x61, 0x62,
    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72,
    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x39, 0x31, 0x30, 0x1A, 0x3F, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B,
    0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x61,
    0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71,
    0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x31, 0x30, 0x22, 0x3F, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A,
    0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
    0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70,
    0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x37, 0x38, 0x39, 0x31, 0x30, 0x2A, 0x3F, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
    0x7A, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x31, 0x30,
  };
  // response
  size_t expected_len = len;
  uint8_t *expected_response = data;
}

void test_repeated_field(void) {
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "hello",
    .project_present = true,
    .project_name = "centre console",
    .project_info = "testing",
    .git_version = "random info",
  };
  query_init(&config);

  // "id" : [ 1, 2, 3 ]
  // "name" : [ "hello", "world" ]
  // "current_project" : [ "pedal board", "centre console" ]
  // "project_info" : ["testing"]
  // "git_version" : []
  size_t query_len = 58;
  uint8_t query[58] = {
    0x08, 0x01, 0x08, 0x02, 0x08, 0x03, 0x12, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x12, 0x05,
    0x77, 0x6F, 0x72, 0x6C, 0x64, 0x1A, 0x0B, 0x70, 0x65, 0x64, 0x61, 0x6C, 0x20, 0x62, 0x6F,
    0x61, 0x72, 0x64, 0x1A, 0x0E, 0x63, 0x65, 0x6E, 0x74, 0x72, 0x65, 0x20, 0x63, 0x6F, 0x6E,
    0x73, 0x6F, 0x6C, 0x65, 0x22, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69, 0x6E, 0x67,
  };

  // bootloader config data in protobuf
  size_t expected_len = 47;
  uint8_t expected[47] = {
    0x08, 0x02, 0x12, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x1A, 0x0E, 0x63, 0x65, 0x6E, 0x74, 0x72,
    0x65, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x6F, 0x6C, 0x65, 0x22, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69,
    0x6E, 0x67, 0x2A, 0x0B, 0x72, 0x61, 0x6E, 0x64, 0x6F, 0x6D, 0x20, 0x69, 0x6E, 0x66, 0x6F,
  };

  prv_test_response(query, query_len, expected, expected_len);
}

void test_empty_query(void) {
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "hello",
    .project_present = true,
    .project_name = "centre console",
    .project_info = "testing",
    .git_version = "random info",
  };
  query_init(&config);

  // "id" : []
  // "name" : []
  // "current_project" : []
  // "project_info" : []
  // "git_version" : []
  // protobuf is empty as there is no data

  // bootloader config data in protobuf
  size_t expected_len = 47;
  uint8_t expected[47] = {
    0x08, 0x02, 0x12, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x1A, 0x0E, 0x63, 0x65, 0x6E, 0x74, 0x72,
    0x65, 0x20, 0x63, 0x6F, 0x6E, 0x73, 0x6F, 0x6C, 0x65, 0x22, 0x07, 0x74, 0x65, 0x73, 0x74, 0x69,
    0x6E, 0x67, 0x2A, 0x0B, 0x72, 0x61, 0x6E, 0x64, 0x6F, 0x6D, 0x20, 0x69, 0x6E, 0x66, 0x6F,
  };
  prv_test_response(NULL, 0, expected, expected_len);
}

void test_no_match(void) {
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "hello",
    .project_present = true,
    .project_name = "centre console",
    .project_info = "testing",
    .git_version = "random info",
  };
  query_init(&config);

  // "id" : [ 1, 2, 3 ]
  // "name" : [ "hello", "world" ]
  // "current_project" : []
  // "project_info" : [ "no match" ]
  // "git_version" : []
  size_t query_len = 30;
  uint8_t query[30] = {
    0x08, 0x01, 0x08, 0x02, 0x08, 0x03, 0x12, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x12, 0x05,
    0x77, 0x6F, 0x72, 0x6C, 0x64, 0x22, 0x08, 0x6E, 0x6F, 0x20, 0x6D, 0x61, 0x74, 0x63, 0x68,
  };

  prv_test_no_response(query, query_len);
}

void test_no_project_on_board(void) {
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "hello",
    .project_present = false,
  };
  query_init(&config);

  query_init(&config);

  // "id" : [ 1, 2, 3 ]
  // "name" : [ "hello", "world" ]
  // "current_project" : []
  // "project_info" : [ "no match" ]
  // "git_version" : []
  size_t query_len = 20;
  uint8_t query[20] = {
    0x08, 0x01, 0x08, 0x02, 0x08, 0x03, 0x12, 0x05, 0x68, 0x65,
    0x6C, 0x6C, 0x6F, 0x12, 0x05, 0x77, 0x6F, 0x72, 0x6C, 0x64,
  };

  size_t expected_len = 9;
  uint8_t expected[9] = {
    0x08, 0x02, 0x12, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F,
  };

  prv_test_response(query, query_len, expected, expected_len);
}

void test_no_project_on_board_no_response(void) {
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "hello",
    .project_present = false,
  };
  query_init(&config);

  // "id" : [ 1, 2, 3 ]
  // "name" : [ "hello", "world" ]
  // "current_project" : []
  // "project_info" : [ "no match" ]
  // "git_version" : []
  size_t query_len = 30;
  uint8_t query[30] = {
    0x08, 0x01, 0x08, 0x02, 0x08, 0x03, 0x12, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x12, 0x05,
    0x77, 0x6F, 0x72, 0x6C, 0x64, 0x22, 0x08, 0x6E, 0x6F, 0x20, 0x6D, 0x61, 0x74, 0x63, 0x68,
  };

  prv_test_no_response(query, query_len);
}
