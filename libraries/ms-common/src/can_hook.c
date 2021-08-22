#include "can_hook.h"

#include <stddef.h>

#include "can.h"
#include "can_fifo.h"
#include "can_msg.h"
#include "event_queue.h"
#include "status.h"

static CanStorage *s_can_storage;
static CanTxHook s_tx_hook = NULL;
static void *s_tx_hook_context = NULL;

void can_hook_init(CanStorage *storage) {
  s_can_storage = storage;
}

StatusCode can_hook_tx_register(CanTxHook hook, void *context) {
  s_tx_hook = hook;
  s_tx_hook_context = context;
  return STATUS_CODE_OK;
}

StatusCode can_hook_rx(const CanMessage *msg) {
  if (s_can_storage == NULL) {
    return STATUS_CODE_UNINITIALIZED;
  }
  status_ok_or_return(can_fifo_push(&s_can_storage->rx_fifo, msg));
  return event_raise(s_can_storage->rx_event, 1);
}

void can_hook_tx_trigger(const CanMessage *msg) {
  if (s_tx_hook != NULL) {
    s_tx_hook(msg, s_tx_hook_context);
  }
}
