#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "interrupt.h"
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

void setup_test(void) {}

void teardown_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  crc32_init();

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
  dispatcher_init(s_board_id);
}

void test_query(void) {
  // test max sized response
  query_init(255, "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910",
             "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910",
             "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910",
             "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910");

  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_QUERY_COMMAND,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len = 0,
    .data = NULL,
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

  TEST_ASSERT_EQUAL(s_board_id, rx_config.data[0]);
  TEST_ASSERT_EQUAL(BOOTLOADER_DATAGRAM_QUERY_RESPONSE, rx_config.dgram_type);
}
