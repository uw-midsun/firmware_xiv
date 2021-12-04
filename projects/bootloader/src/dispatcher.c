#include "dispatcher.h"

#include <stdint.h>
#include <string.h>

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "log.h"

// callbacks and context
static DispatcherCallback s_callback_map[NUM_BOOTLOADER_DATAGRAMS];
static void *s_context_map[NUM_BOOTLOADER_DATAGRAMS];

// setup datagram rx config
static void prv_dispatch(void);
static uint8_t s_destination_nodes[DGRAM_MAX_DEST_NODES_SIZE] = { 0 };
static uint8_t s_data[DGRAM_MAX_DATA_SIZE] = { 0 };
static CanDatagramRxConfig s_datagram_rx = {
  .destination_nodes = s_destination_nodes,
  .data = s_data,
  // .node_id = ?, // node id is set at dispatcher_init
  .rx_cmpl_cb = prv_dispatch,
};

static StatusCode s_response_code;

// when a datagram rx is finished, the corresponding registered callback for that datagram id
// is called with the datagram data and registered context
static void prv_dispatch(void) {
  BootloaderDatagramId id = s_datagram_rx.dgram_type;
  if (id >= NUM_BOOTLOADER_DATAGRAMS) {
    LOG_WARN("Unrecognized Command! %i\n", id);
    return;
  }
  if (s_callback_map[id] == NULL) {
    LOG_WARN("Unregistered Callback for Command: %i\n", id);
    return;
  }
  s_callback_map[id](s_data, s_datagram_rx.data_len, s_context_map[id]);
}

StatusCode dispatcher_init(uint8_t board_id) {
  // reset the callbacks and contexts - mostly for use in tests
  memset(s_callback_map, 0, sizeof(s_callback_map));
  memset(s_context_map, 0, sizeof(s_context_map));

  s_datagram_rx.node_id = board_id;
  // register datagram rx
  bootloader_can_register_handler(can_datagram_rx);
  can_datagram_start_listener(&s_datagram_rx);
  return STATUS_CODE_OK;
}

StatusCode dispatcher_register_callback(BootloaderDatagramId id, DispatcherCallback callback,
                                        void *context) {
  if (id >= NUM_BOOTLOADER_DATAGRAMS) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_callback_map[id] = callback;
  s_context_map[id] = context;
  return STATUS_CODE_OK;
}

void tx_cmpl_cb(void) {
  can_datagram_start_listener(&s_datagram_rx);
}

StatusCode status_response(StatusCode code, CanDatagramExitCb callback) {
  s_response_code = code;
  CanDatagramTxConfig s_response_config = {
    .dgram_type = BOOTLOADER_DATAGRAM_STATUS_RESPONSE,
    .destination_nodes_len = 0,  // client listens to all datagrams
    .destination_nodes = NULL,
    .data_len = 1,
    .data = (uint8_t *)&s_response_code,
    .tx_cb = bootloader_can_transmit,
    .tx_cmpl_cb = callback,
  };
  can_datagram_start_tx(&s_response_config);
  return code;
}
