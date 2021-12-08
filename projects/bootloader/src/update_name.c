#include "update_name.h"

#include <pb_common.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <stdio.h>

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "config.h"
#include "crc32.h"
#include "dispatcher.h"
#include "log.h"
#include "reset.h"
#include "update_name.pb.h"

// Note: Certain functionality will be given to the client script to control
// The client script will make sure only one board is edited per command and
// will make sure the id given is not currently used

static UpdateName s_name_proto = UpdateName_init_zero;

static char name[64];

static CanDatagramTxConfig s_response_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
  .destination_nodes_len = 0,
  .destination_nodes = NULL,
  .data_len = 1,
  .data = NULL,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = tx_cmpl_cb,
};

// Decodes protobuf string to c string
// is inputted to s_name_proto.new_name.funcs.decode
static bool prv_decode_string(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  size_t str_len = MIN(stream->bytes_left, (size_t)64);
  strncpy((char *)*arg, (char *)stream->state, str_len);
  return true;
}

static StatusCode prv_callback_update_name(uint8_t *data, uint16_t data_len, void *context) {
  pb_istream_t stream = pb_istream_from_buffer(data, data_len);
  bool status = pb_decode(&stream, UpdateName_fields, &s_name_proto);

  if (!status) {
    printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
    status_response(STATUS_CODE_INTERNAL_ERROR, tx_cmpl_cb);
    return STATUS_CODE_INTERNAL_ERROR;
  }

  BootloaderConfig previous_config = { 0 };
  config_get(&previous_config);

  BootloaderConfig new_board_config = { .crc32 = 0,
                                        .controller_board_id = previous_config.controller_board_id,
                                        .project_present = previous_config.project_present,
                                        .application_crc32 = previous_config.application_crc32,
                                        .application_size = previous_config.application_size };

  memcpy(new_board_config.controller_board_name, name,
         sizeof(new_board_config.controller_board_name));
  memcpy(new_board_config.project_name, previous_config.project_name,
         sizeof(new_board_config.project_name));
  memcpy(new_board_config.project_info, previous_config.project_info,
         sizeof(new_board_config.project_info));
  memcpy(new_board_config.git_version, previous_config.git_version,
         sizeof(new_board_config.git_version));

  new_board_config.crc32 = crc32_arr((uint8_t *)&new_board_config, sizeof(BootloaderConfig));

  status_ok_or_return(config_commit(&new_board_config));

  // Will send STATUS_CODE_OK back in datagram and
  // upon tx completion will reset software
  status_response(STATUS_CODE_OK, reset);

  // Because of the reset, the return statement will never be reached
  return STATUS_CODE_OK;
}

StatusCode update_name_init(void) {
  s_name_proto.new_name.arg = name;
  s_name_proto.new_name.funcs.decode = prv_decode_string;

  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_UPDATE_NAME, prv_callback_update_name,
                                      NULL);
}
