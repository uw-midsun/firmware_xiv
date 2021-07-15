#include "ping.h"

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can.h"
#include "can_datagram.h"
#include "config.h"

static uint8_t board_id;

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool is_start_message) {
  return bootloader_can_transmit(board_id, data, len, is_start_message);
}

// get the controller_board_id, if the config is not set, return 0xFF (not a valid id)
static uint8_t prv_get_board_id(void) {
  BootloaderConfig blconfig = {
    .controller_board_id = 0xFF,
  };
  config_get(&blconfig);
  return blconfig.controller_board_id;
}

// send a ping response datagram
StatusCode ping_response(void) {
  board_id = prv_get_board_id();

  CanDatagramTxConfig response_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_PING_RESPONSE,
    .destination_nodes_len = 1,
    .destination_nodes = NULL,
    .data_len = 1,
    .data = &board_id,
    .tx_cb = prv_tx_callback,
  };
  can_datagram_start_tx(&response_config);
  return STATUS_CODE_OK;
}
