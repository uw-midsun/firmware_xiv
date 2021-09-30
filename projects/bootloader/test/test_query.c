#include "bootloader_datagram_defs.h"
#include "bootloader_events.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "pb_encode.h"
#include "query.h"
#include "querying.pb.h"
#include "querying_response.pb.h"
#include "test_helpers.h"
#include "unity.h"

#define MAX_STRING_SIZE 64
#define PROTOBUF_MAXSIZE 270  // enough to fit the max possible encoded protobuf

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

static uint8_t s_tx_data[DGRAM_MAX_DATA_SIZE];

static uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE];
static uint8_t s_rx_data[PROTOBUF_MAXSIZE];
static uint16_t s_rx_data_len;

CanDatagramTxConfig s_tx_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_QUERY_COMMAND,
  .destination_nodes_len = 1,
  .destination_nodes = &s_client_id,
  .data = s_tx_data,
  .tx_cmpl_cb = tx_cmpl_cb,
};

CanDatagramRxConfig s_rx_config = {
  .destination_nodes = s_destination_nodes,
  .data = s_rx_data,
  .node_id = 0,  // listen to all
  .rx_cmpl_cb = NULL,
};

static uint8_t s_expected_response[PROTOBUF_MAXSIZE];
static size_t s_expected_len;

// encode a NULL terminated array of strings
static bool prv_encode_string_array(pb_ostream_t *stream, const pb_field_iter_t *field,
                                    void *const *arg) {
  char **array = *arg;
  uint8_t index = 0;
  while (array[index] != NULL) {
    if (!pb_encode_tag_for_field(stream, field) ||
        !pb_encode_string(stream, (uint8_t *)(array[index]), strlen(array[index]))) {
      return false;
    }
    ++index;
  }
  return true;
}

// encode a 0xFF terminated uint8_t array as pb varint
static bool prv_encode_uint8_array(pb_ostream_t *stream, const pb_field_iter_t *field,
                                   void *const *arg) {
  uint8_t *array = *arg;
  uint8_t index = 0;
  while (array[index] != 0xFF) {
    if (!pb_encode_tag_for_field(stream, field) || !pb_encode_varint(stream, array[index])) {
      return false;
    }
    ++index;
  }
  return true;
}

// encode a string to a pb_ostream_t
static bool prv_encode_string(pb_ostream_t *stream, const pb_field_iter_t *field,
                              void *const *arg) {
  const char *str = *arg;
  // add to stream
  if (!pb_encode_tag_for_field(stream, field)) {  // write tag and wire type
    return false;
  }
  return pb_encode_string(stream, (uint8_t *)str, strlen(str));  // write sting
}

static void prv_setup_query(BootloaderConfig *config) {
  query_init(config);
  // set up the response data
  QueryingResponse response = QueryingResponse_init_zero;
  // set QueryingResponse fields
  response.id = config->controller_board_id;
  response.name.arg = config->controller_board_name;
  if (config->project_present) {
    response.current_project.arg = config->project_name;
    response.project_info.arg = config->project_info;
    response.git_version.arg = config->git_version;
  }
  // set the encode functions
  response.name.funcs.encode = prv_encode_string;
  if (config->project_present) {
    response.current_project.funcs.encode = prv_encode_string;
    response.project_info.funcs.encode = prv_encode_string;
    response.git_version.funcs.encode = prv_encode_string;
  }

  // encode protobuf into s_response_config.data
  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_expected_response, PROTOBUF_MAXSIZE);
  TEST_ASSERT_MESSAGE(pb_encode(&pb_ostream, QueryingResponse_fields, &response),
                      "failed to encode rx pb");
  s_expected_len = pb_ostream.bytes_written;
  LOG_DEBUG("encode rx complete\n");
}

static void prv_encode_query(uint8_t *id, char **name, char **current_project, char **project_info,
                             char **git_version) {
  Querying query = Querying_init_zero;

  query.id.funcs.encode = prv_encode_uint8_array;
  query.name.funcs.encode = prv_encode_string_array;
  query.current_project.funcs.encode = prv_encode_string_array;
  query.project_info.funcs.encode = prv_encode_string_array;
  query.git_version.funcs.encode = prv_encode_string_array;

  query.id.arg = id;
  query.name.arg = name;
  query.current_project.arg = current_project;
  query.project_info.arg = project_info;
  query.git_version.arg = git_version;

  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_tx_data, DGRAM_MAX_DATA_SIZE);
  TEST_ASSERT_MESSAGE(pb_encode(&pb_ostream, Querying_fields, &query), "failed to encode tx pb");
  s_tx_config.data_len = pb_ostream.bytes_written;
  LOG_DEBUG("encode tx complete\n");
}

static void prv_test_query_response() {
  dgram_helper_mock_tx_datagram(&s_tx_config);
  dgram_helper_mock_rx_datagram(&s_rx_config);

  TEST_ASSERT_EQUAL(DATAGRAM_STATUS_RX_COMPLETE, can_datagram_get_status());

  TEST_ASSERT_EQUAL(BOOTLOADER_DATAGRAM_QUERY_RESPONSE, s_rx_config.dgram_type);
  TEST_ASSERT_EQUAL(s_expected_len, s_rx_config.data_len);
  for (uint16_t i = 0; i < s_rx_config.data_len; ++i) {
    TEST_ASSERT_EQUAL(s_rx_config.data[i], s_expected_response[i]);
  }
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
  prv_setup_query(&config);

  uint8_t id[] = { 2, 1, 2, 3, 0xFF };
  char *name[] = { "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910", NULL };
  char *cur_proj[] = { "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910", NULL };
  char *proj_info[] = { "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910", NULL };
  char *git_ver[] = { "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz12345678910", NULL };

  prv_encode_query(id, name, cur_proj, proj_info, git_ver);
  prv_test_query_response();
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
  prv_setup_query(&config);

  uint8_t id[] = { 1, 2, 3, 0xFF };
  char *name[] = { "hello", "world", NULL };
  char *cur_proj[] = { "pedal board", "centre console", NULL };
  char *proj_info[] = { "testing", NULL };
  char *git_ver[] = { NULL };

  prv_encode_query(id, name, cur_proj, proj_info, git_ver);
  prv_test_query_response();
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
  prv_setup_query(&config);

  uint8_t id[] = { 0xFF };
  char *name[] = { NULL };
  char *cur_proj[] = { NULL };
  char *proj_info[] = { NULL };
  char *git_ver[] = { NULL };

  prv_encode_query(id, name, cur_proj, proj_info, git_ver);
  prv_test_query_response();
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
  prv_setup_query(&config);

  uint8_t id[] = { 1, 2, 3, 0xFF };
  char *name[] = { "hello", "world", NULL };
  char *cur_proj[] = { NULL };
  char *proj_info[] = { "no match", NULL };
  char *git_ver[] = { NULL };

  prv_encode_query(id, name, cur_proj, proj_info, git_ver);
  dgram_helper_mock_tx_datagram(&s_tx_config);
  dgram_helper_assert_no_response();
}

void test_no_project_on_board(void) {
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "hello",
    .project_present = false,
  };
  prv_setup_query(&config);

  uint8_t id[] = { 1, 2, 3, 0xFF };
  char *name[] = { "hello", "world", NULL };
  char *cur_proj[] = { NULL };
  char *proj_info[] = { NULL };
  char *git_ver[] = { NULL };

  prv_encode_query(id, name, cur_proj, proj_info, git_ver);
  prv_test_query_response();
}

void test_no_project_on_board_no_response(void) {
  BootloaderConfig config = {
    .controller_board_id = s_board_id,
    .controller_board_name = "hello",
    .project_present = false,
  };
  prv_setup_query(&config);

  uint8_t id[] = { 1, 2, 3, 0xFF };
  char *name[] = { "hello", "world", NULL };
  char *cur_proj[] = { NULL };
  char *proj_info[] = { "no match", NULL };
  char *git_ver[] = { NULL };

  prv_encode_query(id, name, cur_proj, proj_info, git_ver);
  dgram_helper_mock_tx_datagram(&s_tx_config);
  dgram_helper_assert_no_response();
}
