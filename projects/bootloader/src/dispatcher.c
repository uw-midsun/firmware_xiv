#include "dispatcher.h"

#include <stdint.h>
#include <string.h>

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"

#define MAX_DEST_NODES_SIZE_BYTES 255
#define MAX_DATA_SIZE_BYTES 2048

// callbacks and context
static DispatcherCallback s_callback_map[NUM_BOOTLOADER_DATAGRAMS] = { NULL };
static void *s_context_map[NUM_BOOTLOADER_DATAGRAMS] = { NULL };

// setup datagram rx config
static void prv_dispatch(void);
static uint8_t s_destination_nodes[MAX_DEST_NODES_SIZE_BYTES] = { 0 };
static uint8_t s_data[MAX_DATA_SIZE_BYTES] = { 0 };
static CanDatagramRxConfig datagram_rx = {
  .destination_nodes = s_destination_nodes,
  .data = s_data,
  // .node_id = 0, // node id is set at dispatcher_init
  .rx_cmpl_cb = prv_dispatch,
};

// when a datagram rx is finished, the corresponding registered callback for that datagram id
// is called with the datagram data and registered context
static void prv_dispatch(void) {
  BootloaderDatagramId id = datagram_rx.dgram_type;
  if (id < NUM_BOOTLOADER_DATAGRAMS && s_callback_map[id] != NULL) {
    s_callback_map[id](s_data, datagram_rx.data_len, s_context_map[id]);
  }
}

StatusCode dispatcher_init(uint8_t board_id) {
  // reset the callbacks and contexts - mostly for use in tests
  memset(s_callback_map, 0, sizeof(s_callback_map));
  memset(s_context_map, 0, sizeof(s_context_map));

  datagram_rx.node_id = board_id;
  // register datagram rx
  bootloader_can_register_handler(can_datagram_rx);
  can_datagram_start_listener(&datagram_rx);
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
  can_datagram_start_listener(&datagram_rx);
}
