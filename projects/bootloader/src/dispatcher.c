#include "dispatcher.h"

#include <stdint.h>
#include <string.h>

#include "bootloader_can.h"
#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"

#define MAX_DEST_NODES_SIZE_BYTES 255
#define MAX_DATA_SIZE_BYTES 2048
// callbacks and context
static DispatcherCallback s_callback_map[NUM_BOOTLOADER_DATAGRAMS] = { NULL };
static void *s_context_map[NUM_BOOTLOADER_DATAGRAMS] = { NULL };

// setup datagram rx config
static void prv_dispatch(void);
static uint8_t s_desination_nodes[MAX_DEST_NODES_SIZE_BYTES] = { 0 };
static uint8_t s_data[MAX_DATA_SIZE_BYTES] = { 0 };
static CanDatagramRxConfig datagram_rx = {
  .destination_nodes = s_desination_nodes,
  .data = s_data,
  .node_id = 0,  // should be the id of this controller board.
  .rx_cmpl_cb = prv_dispatch,
};

static void prv_dispatch() {
  BootloaderDatagramId id = datagram_rx.dgram_type;
  if (id < NUM_BOOTLOADER_DATAGRAMS && s_callback_map[id] != NULL) {
    s_callback_map[id](s_data, datagram_rx.data_len, s_context_map[id]);
  }
}

static StatusCode prv_bootloader_can_callback(const CanMessage *msg, void *context) {
  if (msg->type == CAN_MSG_TYPE_ACK) {
    can_datagram_rx(NULL, 0, true);
  } else if (msg->type == CAN_MSG_TYPE_DATA) {
    uint8_t data[8];
    can_unpack_impl_u8(msg, msg->dlc, &data[0], &data[1], &data[2], &data[3], &data[4], &data[5],
                       &data[6], &data[7]);
    can_datagram_rx(data, msg->dlc, false);
  }
  return STATUS_CODE_OK;
}

StatusCode dispatcher_init(void) {
  // reset the callbacks and contexts - mostly for use in tests
  memset(s_callback_map, 0, sizeof(s_callback_map));
  memset(s_context_map, 0, sizeof(s_context_map));

  // register datagram rx
  bootloader_can_register_handler(prv_bootloader_can_callback, NULL);
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
