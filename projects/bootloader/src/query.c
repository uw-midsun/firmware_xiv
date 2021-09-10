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

#define PROTOBUF_MAXSIZE 512

static QueryingResponse s_response = QueryingResponse_init_zero;
static uint8_t s_response_encoded[PROTOBUF_MAXSIZE];

static CanDatagramTxConfig s_response_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_QUERY_RESPONSE,
  .destination_nodes_len = 0,  // client listens to all datagrams
  .destination_nodes = NULL,
  .data_len = 0,
  .data = s_response_encoded,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = tx_cmpl_cb,
};

static bool s_encode_string(pb_ostream_t *stream, const pb_field_iter_t *field, void *const *arg) {
  const char *str = (const char *)(*arg);

  if (!pb_encode_tag_for_field(stream, field)) return false;
  return pb_encode_string(stream, (uint8_t *)str, strlen(str));
}

static void prv_check_query(uint8_t *data, uint16_t data_len, void *context) {
  pb_istream_t pb_istream = pb_istream_from_buffer(data, data_len);
  // pb_decode(pb_istream, Querying class?, &s_query);

  // for (query : s_query) {
  //   if (pattern matches) {
  //     can_datagram_start_tx(s_response_config);
  //     return;
  //   }
  // }

  // for (field in fields) {
  //   if (empty) {
  //     continue;
  //   } else if (this in field) {
  //     continue;
  //   } else {
  //     return;
  //   }
  // }
  // send_response;
}

StatusCode query_init(BootloaderConfig *config) {
  s_response.id = config->controller_board_id;
  s_response.name.arg = config->controller_board_name;
  if (config->project_present) {
    s_response.current_project.arg = config->project_name;
    s_response.project_info.arg = config->project_info;
    s_response.git_version.arg = config->git_version;
  }

  // set the pb_callback_t for string fields
  s_response.name.funcs.encode = s_encode_string;
  s_response.current_project.funcs.encode = s_encode_string;
  s_response.project_info.funcs.encode = s_encode_string;
  s_response.git_version.funcs.encode = s_encode_string;
  // encode protobuf into s_response_config.data
  pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_response_encoded, PROTOBUF_MAXSIZE);
  pb_encode(&pb_ostream, QueryingResponse_fields, &s_response);
  s_response_config.data_len = pb_ostream.bytes_written;

  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_QUERY_COMMAND, prv_check_query, NULL);
}
