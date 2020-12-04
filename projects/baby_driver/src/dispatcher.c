#include "dispatcher.h"

#include <stdint.h>
#include <string.h>

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"

static DispatcherCallback s_callback_map[NUM_BABYDRIVER_MESSAGES] = { NULL };
static void *s_context_map[NUM_BABYDRIVER_MESSAGES] = { NULL };

static void prv_call_callback(BabydriverMessageId id, uint8_t data[8]) {
  bool tx_result = true;
  if (s_callback_map[id] != NULL) {
    StatusCode status = s_callback_map[id](data, s_context_map[id], &tx_result);
    if (tx_result) {
      CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_STATUS, status, 0, 0, 0, 0, 0, 0);
    }
  }
}

static StatusCode prv_dispatch(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  BabydriverMessageId id = msg->data_u8[0];
  if (id < NUM_BABYDRIVER_MESSAGES && s_callback_map[id] != NULL) {
    prv_call_callback(id, msg->data_u8);
  }
  return STATUS_CODE_OK;
}

StatusCode dispatcher_init(void) {
  // reset the callbacks and contexts - mostly for use in tests
  memset(s_callback_map, 0, sizeof(s_callback_map));
  memset(s_context_map, 0, sizeof(s_context_map));

  // register our callback
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_BABYDRIVER, prv_dispatch, NULL);
}

StatusCode dispatcher_register_callback(BabydriverMessageId id, DispatcherCallback callback,
                                        void *context) {
  if (id >= NUM_BABYDRIVER_MESSAGES) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_callback_map[id] = callback;
  s_context_map[id] = context;
  return STATUS_CODE_OK;
}
