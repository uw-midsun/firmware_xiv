#include "update_id.h"
#include "update_id.pb.h"

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "test_helpers.h"
#include "unity.h"

static uint8_t s_board_id = 1;
#define NODE_CHANGE_VALUE 3

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

static uint8_t s_tx_data[DGRAM_MAX_DATA_SIZE];

static uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE];
static uint8_t s_rx_data[DGRAM_MAX_DATA_SIZE];

static CanDatagramTxConfig s_tx_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_UPDATE_ID,
  .destination_nodes_len = 1,
  .destination_nodes = &s_board_id,
  .data_len = 0,      // 0 is place holder for real value that will be set
  .data = s_tx_data,  // Make fake proto to send over the tx
  .tx_cmpl_cb = tx_cmpl_cb,
};

static CanDatagramRxConfig s_rx_config = {
  .destination_nodes = s_destination_nodes,
  .data = s_rx_data,
  .node_id = 0,  // listen to all
  .rx_cmpl_cb = NULL,
};

static void prv_encode_id(uint8_t id) {
  UpdateId id_proto = UpdateId_init_zero;

  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_tx_data, DGRAM_MAX_DATA_SIZE);

  id_proto.id = id;

  TEST_ASSERT_MESSAGE(pb_encode(&pb_ostream, UpdateId_fields, &id_proto), "failed to encode tx pb");
  s_tx_config.data_len = pb_ostream.bytes_written;
  LOG_DEBUG("encode tx complete\n");
}

void setup_test(void) {
  TEST_ASSERT_OK(dispatcher_init(s_board_id));
  TEST_ASSERT_OK(update_id_init());

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
}

void teardown_test(void) {}

void test_update_id(void) {
  prv_encode_id(NODE_CHANGE_VALUE);

  dgram_helper_mock_tx_datagram(&s_tx_config);
  dgram_helper_mock_rx_datagram(&s_rx_config);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_rx_config.data[0]);
}

void test_corrupt_proto(void) {
  // figure out how to make a bad proto
  // "You can probably feed random bytes into it and it should fail decoding"
}

// and I could also test to see if a failing proto works
// I will need to mock reset

StatusCode TEST_MOCK(reset)(void) {
  // This function mocks reset (does nothing)
  LOG_DEBUG("Mocked reset executed \n");

  return STATUS_CODE_OK;
}
