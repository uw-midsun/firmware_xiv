#include "update_name.h"
#include "update_name.pb.h"

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>

#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "flash.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "test_helpers.h"
#include "unity.h"

static uint8_t s_board_id = 1;
static const char NAME_CHANGE_VALUE[] = "name change";
static UpdateName s_name_proto = UpdateName_init_zero;

#define MAX_STRING_SIZE 64

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
  .dgram_type = BOOTLOADER_DATAGRAM_UPDATE_NAME,
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

static BootloaderConfig reset_config = { .crc32 = 1,
                                         .controller_board_id = 1,
                                         .controller_board_name = "a",
                                         .project_present = true,
                                         .project_name = "a",
                                         .project_info = "a",
                                         .git_version = "a",
                                         .application_crc32 = 1,
                                         .application_size = 1 };

static size_t prv_strnlen(const char *str, size_t maxlen) {
  size_t i;
  for (i = 0; i < maxlen; ++i) {
    if (str[i] == '\0') {
      break;
    }
  }
  return i;
}

// encode a string to a pb_ostream_t
static bool prv_encode_string(pb_ostream_t *stream, const pb_field_iter_t *field,
                              void *const *arg) {
  const char *str = *arg;
  // add to stream
  if (!pb_encode_tag_for_field(stream, field)) {  // write tag and wire type
    return false;
  }
  return pb_encode_string(stream, (uint8_t *)str,
                          prv_strnlen(str, MAX_STRING_SIZE));  // write string
}

// This function encodes a protobuf with the input name
static void prv_encode_name(char name[]) {
  // set the encode functions
  s_name_proto.new_name.funcs.encode = prv_encode_string;

  s_name_proto.new_name.arg = name;

  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_tx_data, DGRAM_MAX_DATA_SIZE);

  TEST_ASSERT_MESSAGE(pb_encode(&pb_ostream, UpdateName_fields, &s_name_proto),
                      "failed to encode tx pb");
  s_tx_config.data_len = pb_ostream.bytes_written;
  LOG_DEBUG("encode tx complete\n");
}

void setup_test(void) {
  flash_init();
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  crc32_init();
  config_init();

  dispatcher_init(s_board_id);
  TEST_ASSERT_OK(update_name_init());

  ms_test_helper_datagram_init(&s_test_can_storage, &s_test_can_settings, s_board_id,
                               &s_test_datagram_settings);
  config_commit(&reset_config);
}

void teardown_test(void) {}

void test_update_name(void) {
  // Test determines if config name is changed to NAME_CHANGE_VALUE

  // Encodes the protobuf
  prv_encode_name(NAME_CHANGE_VALUE);

  // Sends tx datagram
  dgram_helper_mock_tx_datagram(&s_tx_config);

  // Receives rx datagram from update_name.c
  dgram_helper_mock_rx_datagram(&s_rx_config);

  // Verifies datagram rx status
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());
  // Verifies that STATUS_CODE_OK is returned from update_name.c
  TEST_ASSERT_EQUAL(STATUS_CODE_OK, s_rx_config.data[0]);

  BootloaderConfig current_config = { 0 };
  config_get(&current_config);
  TEST_ASSERT_EQUAL_STRING(NAME_CHANGE_VALUE, current_config.controller_board_name);
}

void test_corrupt_proto(void) {
  // Test determines if protobuf is corrupted, update_name will fail
  uint8_t random_bytes[DGRAM_MAX_DATA_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  s_tx_config.data = random_bytes;

  // Sends tx datagram
  dgram_helper_mock_tx_datagram(&s_tx_config);
  // Receives rx datagram from update_name.c
  dgram_helper_mock_rx_datagram(&s_rx_config);

  // Verifies datagram rx status
  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

  // Verifies that STATUS_CODE_INTERNAL_ERROR is returned from update_name.c
  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR, s_rx_config.data[0]);
}

StatusCode TEST_MOCK(reset)(void) {
  // This function mocks reset (does nothing)
  LOG_DEBUG("Mocked reset executed \n");

  return STATUS_CODE_OK;
}
