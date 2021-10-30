#include "update_id.h"

#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "ms_test_helper_datagram.h"
#include "test_helpers.h"

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

static uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE];
static uint8_t s_rx_data[DGRAM_MAX_DATA_SIZE];

void setup_test(void) {
  TEST_ASSERT_OK(dispatcher_init(s_board_id));
  TEST_ASSERT_OK(update_id_init());

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
}

void teardown_test(void) {}

void test_update_id(void) {
  CanDatagramTxConfig tx_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_UPDATE_ID,
    .destination_nodes_len = 1,
    .destination_nodes = &s_board_id,
    .data_len =,  // find how much space a proto will take
    .data =,      // Make fake proto to send over the tx
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
}

// figure out if I need to mock protos, this would fix my issue with creating a fake proto,
// and I could also test to see if a failing proto works
// I will need to mock reset
