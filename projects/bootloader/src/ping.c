#include "ping.h"

#include <stdint.h>

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "config.h"
#include "dispatcher.h"

// response config datagram setup
static uint8_t s_board_id;
static CanDatagramTxConfig s_response_config = {
  .dgram_type = BOOTLOADER_DATAGRAM_PING_RESPONSE,
  .destination_nodes_len = 0,  // client listens to all datagrams
  .destination_nodes = NULL,
  .data_len = 1,
  .data = &s_board_id,
  .tx_cb = bootloader_can_transmit,
  .tx_cmpl_cb = tx_cmpl_cb,
};

static StatusCode prv_ping_response(uint8_t *data, uint16_t data_len, void *context) {
  return can_datagram_start_tx(&s_response_config);
}

StatusCode ping_init(uint8_t board_id) {
  s_board_id = board_id;
  return dispatcher_register_callback(BOOTLOADER_DATAGRAM_PING_COMMAND, prv_ping_response, NULL);
}
