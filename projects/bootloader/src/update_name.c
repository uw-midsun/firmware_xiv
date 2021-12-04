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
#include "reset.h"
#include "update_name.pb.h"

// Note: Certain functionality will be given to the client script to control
// The client script will make sure only one board is edited per command and
// will make sure the id given is not currently used

static UpdateName name_proto = UpdateName_init_zero;

static CanDatagramTxConfig s_response_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
  .destination_nodes_len = 0,
  .destination_nodes = NULL,
  .data_len = 1,
  .data = NULL,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = tx_cmpl_cb,
};

/*static bool data_string_decode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
used for decoding string char buf[64];

  printf("Decoding data\n");
  if (!pb_decode(stream, field, buf)) {
    printf("Failed\n");
    return false;
  }

  printf("decode data success\n");
  return true;
}*/

static bool prv_decode_string(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
  size_t str_len = MIN(stream->bytes_left, (size_t)64);
  strncpy((char *)*arg, (char *)stream->state, str_len);
  return true;
}

static StatusCode prv_callback_update_name(uint8_t *data, uint16_t data_len, void *context) {
  pb_istream_t stream = pb_istream_from_buffer(data, data_len);
  bool status = pb_decode(&stream, UpdateName_fields, &name_proto);

  if (!status) {
    printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
    uint8_t response_data = STATUS_CODE_INTERNAL_ERROR;
    s_response_config.data = &response_data;
    return can_datagram_start_tx(&s_response_config);
  }

  BootloaderConfig previous_config = { 0 };
  config_get(&previous_config);

  BootloaderConfig new_board_config = { .crc32 = 0,
                                        .controller_board_id = previous_config.controller_board_id,
                                        .project_present = previous_config.project_present,
                                        .application_crc32 = previous_config.application_crc32,
                                        .application_size = previous_config.application_size };

  // FIX name issue where string can't be c array
  // char new_name[64] = name_proto.new_name;
  // char new_name[64];
  // strcpy (new_name, name_proto.new_name);
  char new_name[64] = "name";

  memcpy(new_board_config.controller_board_name, new_name,
         sizeof(new_board_config.controller_board_name));
  memcpy(new_board_config.project_name, previous_config.project_name,
         sizeof(new_board_config.project_name));
  memcpy(new_board_config.project_info, previous_config.project_info,
         sizeof(new_board_config.project_info));
  memcpy(new_board_config.git_version, previous_config.git_version,
         sizeof(new_board_config.git_version));

  new_board_config.crc32 = crc32_arr((uint8_t *)&new_board_config, sizeof(BootloaderConfig));

  config_commit(&new_board_config);

  uint8_t response_data = STATUS_CODE_INTERNAL_ERROR;
  s_response_config.data = &response_data;
  can_datagram_start_tx(&s_response_config);

  // Software reset
  reset();

  // Because of the reset, the return statement will never be reached
  return STATUS_CODE_OK;
}

StatusCode update_name_init(void) {
  name_proto.new_name.funcs.decode = prv_decode_string;

  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_UPDATE_NAME, prv_callback_update_name,
                                      NULL);
}
