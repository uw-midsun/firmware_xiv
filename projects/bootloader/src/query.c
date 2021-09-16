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
#include "log.h"
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

typedef struct Search {
  void *field;
  size_t size;
  CompareResult result;
} Search;

static Search s_search[NUM_QUERY_FIELDS];

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
  Search *search_field = s_search + field->index;
  uint8_t *id = (uint8_t *)search_field->field;
  uint64_t query_id;
  if (!pb_decode_varint(stream, &query_id)) {
    return false;
  }
  if (query_id == *id) {
    search_field->result = FOUND;
  } else if (search_field->result != FOUND) {
    search_field->result = NOT_FOUND;
  }
  return true;
}

// compares the query field to the board field directly without storying the query
static bool prv_decode_cmp_string(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  Search *search_field = s_search + field->index;
  // found in a previous query
  if (search_field->result == FOUND) {
    return true;
  }
  // compare the field to the board field
  if (search_field->size != stream->bytes_left) {
    search_field->result = NOT_FOUND;
  } else if (strncmp(search_field->field, stream->state, stream->bytes_left) == 0) {
    search_field->result = FOUND;
  } else {
    search_field->result = NOT_FOUND;
  }
  return true;
}

static StatusCode prv_check_query(uint8_t *data, uint16_t data_len, void *context) {
  for (int i = 0; i < NUM_QUERY_FIELDS; ++i) {
    s_search[i].result = NON_EXISTANT;
  }

  Querying query = Querying_init_zero;

  query.id.funcs.decode = prv_decode_cmp_varint;
  query.name.funcs.decode = prv_decode_cmp_string;
  query.current_project.funcs.decode = prv_decode_cmp_string;
  query.project_info.funcs.decode = prv_decode_cmp_string;
  query.git_version.funcs.decode = prv_decode_cmp_string;

  pb_istream_t pb_istream = pb_istream_from_buffer(data, data_len);
  pb_decode(&pb_istream, Querying_fields, &query);

  for (int i = 0; i < NUM_QUERY_FIELDS; ++i) {
    if (s_search[i].result == NOT_FOUND) {
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
  // add to search target
  Search *search_field = s_search + field->index;
  search_field->field = str;
  search_field->size = strlen(str);
  // add to stream
  if (str == NULL) {  // if the field is not set, don't write anything
    return true;
  }
  if (!pb_encode_tag_for_field(stream, field)) {  // write tag and wire type
    return false;
  }
  return pb_encode_string(stream, (uint8_t *)str, strlen(str));  // write sting
}

StatusCode prv_setup_querying_response(uint8_t *id, char *name, char *current_project,
                                       char *project_info, char *git_version) {
  QueryingResponse response = QueryingResponse_init_zero;
  // set QueryingResponse fields
  response.id = *id;
  response.name.arg = name;
  response.current_project.arg = current_project;
  response.project_info.arg = project_info;
  response.git_version.arg = git_version;
  // set the encode functions
  response.name.funcs.encode = prv_encode_string;
  response.current_project.funcs.encode = prv_encode_string;
  response.project_info.funcs.encode = prv_encode_string;
  response.git_version.funcs.encode = prv_encode_string;

  // encode protobuf into s_response_config.data
  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_datagram_data, PROTOBUF_MAXSIZE);
  if (!pb_encode(&pb_ostream, QueryingResponse_fields, &response)) {
    return STATUS_CODE_INTERNAL_ERROR;  // pb encode failed
  }

  // set datagram length to protobuf length
  s_response_config.data_len = pb_ostream.bytes_written;

  // set the field for id, not set during protobuf encoding
  s_search[0].field = id;

  return STATUS_CODE_OK;
}

StatusCode query_init(uint8_t *id, char *name, char *current_project, char *project_info,
                      char *git_version) {
  prv_setup_querying_response(id, name, current_project, project_info, git_version);
  LOG_DEBUG("data len: %i", s_response_config.data_len);
  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_QUERY_COMMAND, prv_check_query, NULL);
}
