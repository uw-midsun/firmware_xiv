#include "ping.h"

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "config.h"
#include "dispatcher.h"

// this function needs to be declared for response_config datagram
static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool is_start_message);
// response_config datagram setup
static uint8_t board_id;
static uint8_t destination_node = 0;
static CanDatagramTxConfig response_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_PING_RESPONSE,
  .destination_nodes_len = 1,
  .destination_nodes = &destination_node,
  .data_len = 1,
  .data = &board_id,
  .tx_cb = prv_tx_callback,
};

static StatusCode prv_tx_callback(uint8_t *data, size_t len, bool is_start_message) {
  return bootloader_can_transmit(board_id, data, len, is_start_message);
}

static StatusCode prv_ping_response(uint8_t data[2048], uint16_t data_len, void *context) {
  return can_datagram_start_tx(&response_config);
}

// get the controller_board_id, default return 0xFF if the config is not set (not a valid id)
static uint8_t prv_get_board_id(void) {
  BootloaderConfig blconfig = {
    .controller_board_id = 0xFF,
  };
  config_get(&blconfig);
  return blconfig.controller_board_id;
}

StatusCode ping_init(void) {
  board_id = prv_get_board_id();
  dispatcher_register_callback(BOOTLOADER_DATAGRAM_PING_COMMAND, prv_ping_response, NULL);
  return STATUS_CODE_OK;
}
