#include "query.h"

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <string.h>

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "config.h"
#include "dispatcher.h"
#include "querying.pb.h"
#include "querying_response.pb.h"

#define MAX_STRING_SIZE 64
#define PROTOBUF_MAXSIZE 270  // enough to fit the max possible encoded protobuf
#define NUM_QUERY_FIELDS 5

typedef enum {
  NOT_FOUND = 0,
  FOUND,
  NON_EXISTANT,
} CompareResult;

static void *s_targets[NUM_QUERY_FIELDS];
static CompareResult s_results[NUM_QUERY_FIELDS];
static Querying s_querying = Querying_init_zero;

static uint8_t s_datagram_data[PROTOBUF_MAXSIZE];

static CanDatagramTxConfig s_response_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_QUERY_RESPONSE,
  .destination_nodes_len = 0,  // client listens to all datagrams
  .destination_nodes = NULL,
  .data_len = 0,
  .data = s_datagram_data,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = tx_cmpl_cb,
};

static bool prv_decode_cmp_varint(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  if (s_results[field->index] == FOUND) {
    return true;
  }
  s_results[field->index] = NOT_FOUND;

  uint8_t *target = s_targets[field->index];
  uint64_t incoming;
  if (!pb_decode_varint(stream, &incoming)) {
    return false;
  }
  if (incoming == *target) {
    s_results[field->index] = FOUND;
  }
  return true;
}

// compares the query field to the board field directly without storying the query
static bool prv_decode_cmp_string(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  if (s_results[field->index] == FOUND) {
    return true;
  }
  s_results[field->index] = NOT_FOUND;

  char *target = (char *)s_targets[field->index];
  if (target == NULL) {
    return true;
  }
  // compare the field to the board field
  if (strncmp(target, stream->state, stream->bytes_left) == 0) {
    s_results[field->index] = FOUND;
  }
  pb_read(stream, NULL, stream->bytes_left);
  return true;
}

static StatusCode prv_check_query(uint8_t *data, uint16_t data_len, void *context) {
  for (int i = 0; i < NUM_QUERY_FIELDS; ++i) {
    s_results[i] = NON_EXISTANT;
  }

  pb_istream_t pb_istream = pb_istream_from_buffer(data, data_len);
  pb_decode(&pb_istream, Querying_fields, &s_querying);

  for (int i = 0; i < NUM_QUERY_FIELDS; ++i) {
    if (s_results[i] == NOT_FOUND) {
      // do not match the query
      return STATUS_CODE_OK;
    }
  }
  return can_datagram_start_tx(&s_response_config);
}

// encode a string to a pb_ostream_t
static bool prv_encode_string(pb_ostream_t *stream, const pb_field_iter_t *field,
                              void *const *arg) {
  const char *str = (const char *)(*arg);
  // add to stream
  if (!pb_encode_tag_for_field(stream, field)) {  // write tag and wire type
    return false;
  }
  return pb_encode_string(stream, (uint8_t *)str, strlen(str));  // write sting
}

StatusCode query_init(BootloaderConfig *config) {
  // set up the search variables to compare the query to
  // tag - 1 as the tags start at index 0
  s_targets[Querying_id_tag - 1] = &config->controller_board_id;
  s_targets[Querying_name_tag - 1] = config->controller_board_name;
  if (config->project_present) {
    s_targets[Querying_current_project_tag - 1] = config->project_name;
    s_targets[Querying_project_info_tag - 1] = config->project_info;
    s_targets[Querying_git_version_tag - 1] = config->git_version;
  }
  // querying's decode functions
  s_querying.id.funcs.decode = prv_decode_cmp_varint;
  s_querying.name.funcs.decode = prv_decode_cmp_string;
  s_querying.current_project.funcs.decode = prv_decode_cmp_string;
  s_querying.project_info.funcs.decode = prv_decode_cmp_string;
  s_querying.git_version.funcs.decode = prv_decode_cmp_string;

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
  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_datagram_data, PROTOBUF_MAXSIZE);
  if (!pb_encode(&pb_ostream, QueryingResponse_fields, &response)) {
    return STATUS_CODE_INTERNAL_ERROR;  // pb encode failed
  }

  // set datagram length to protobuf length
  s_response_config.data_len = pb_ostream.bytes_written;

  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_QUERY_COMMAND, prv_check_query, NULL);
}
