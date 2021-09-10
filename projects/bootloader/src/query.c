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

// #include "querying.pb.h"
// #include "querying_response.pb.h"

#define PROTOBUF_MAXSIZE 512

// struct QueryingResponse s_query_res;

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

// static bool s_encode_string(pb_ostream_t *stream,
// const pb_field_iter_t *field, void *const *arg) {
//   const char *str = (const char *)(*arg);

//   if (!pb_encode_tag_for_field(stream, field)) return false;
//   return pb_encode_string(stream, (uint8_t *)str, strlen(str));
// }

// static void prv_check_query(uint8_t *data, uint16_t data_len, void *context) {
// pb_istream_t pb_istream = pb_istream_from_buffer(data, data_len);
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
// }

StatusCode query_init(BootloaderConfig *config) {
  // QueryingResponse s_query = {
  //   .id = config->controller_board_id,
  //   .name = { s_encode_string },
  //   .project_info = s_encode_string,
  //   .current_project = s_encode_string,
  //   .git_version = s_encode_string,
  // };
  // // encode protobuf into s_response_config.data
  // pb_ostream_t pb_ostream = pb_ostream_from_buffer(s_response_encoded, PROTOBUF_MAXSIZE);
  // pb_encode(&pb_ostream, &QueryingResponse_msg, &s_query);
  // s_response_config.data_len = pb_ostream.bytes_written;

  // for (int i = 0; i < pb_ostream.bytes_written; ++i) {
  //   LOG_DEBUG("%x ", s_response_encoded[i]);
  // }
  return STATUS_CODE_OK;
  // return dispatcher_register_callback(BOOTLOADER_DATAGRAM_QUERY_COMMAND, prv_check_query, NULL);
}
