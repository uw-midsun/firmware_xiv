#include "can_rx.h"
#include <stdlib.h>
#include <string.h>

static int prv_handler_comp(const void *a, const void *b) {
  const CanRxHandler *x = a;
  const CanRxHandler *y = b;

  return x->msg_id - y->msg_id;
}

StatusCode can_rx_init(CanRxHandlers *rx_handlers, CanRxHandler *handler_storage,
                       size_t num_handlers) {
  memset(rx_handlers, 0, sizeof(*rx_handlers));
  memset(handler_storage, 0, sizeof(*handler_storage) * num_handlers);

  rx_handlers->storage = handler_storage;
  rx_handlers->num_handlers = 0;
  rx_handlers->max_handlers = num_handlers;

  return STATUS_CODE_OK;
}

StatusCode can_rx_register_default_handler(CanRxHandlers *rx_handlers, CanRxHandlerCb handler,
                                           void *context) {
  StatusCode ret = can_rx_register_handler(rx_handlers, CAN_MSG_INVALID_ID, handler, context);

  if (ret == STATUS_CODE_OK) {
    rx_handlers->default_handler = can_rx_get_handler(rx_handlers, CAN_MSG_INVALID_ID);
  }

  return ret;
}

StatusCode can_rx_register_handler(CanRxHandlers *rx_handlers, CanMessageId msg_id,
                                   CanRxHandlerCb handler, void *context) {
  if (rx_handlers->num_handlers == rx_handlers->max_handlers) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN RX handlers full");
  } else if (can_rx_get_handler(rx_handlers, msg_id) != NULL) {
    return status_msg(STATUS_CODE_RESOURCE_EXHAUSTED, "CAN RX handler already registered");
  }

  rx_handlers->storage[rx_handlers->num_handlers++] = (CanRxHandler){
    .msg_id = msg_id,     //
    .callback = handler,  //
    .context = context,
  };

  qsort(rx_handlers->storage, rx_handlers->num_handlers, sizeof(*rx_handlers->storage),
        prv_handler_comp);

  return STATUS_CODE_OK;
}

CanRxHandler *can_rx_get_handler(CanRxHandlers *rx_handlers, CanMessageId msg_id) {
  const CanRxHandler key = {
    .msg_id = msg_id,
  };
  CanRxHandler *handler = bsearch(&key, rx_handlers->storage, rx_handlers->num_handlers,
                                  sizeof(*rx_handlers->storage), prv_handler_comp);

  if (handler == NULL && rx_handlers->default_handler != NULL) {
    return rx_handlers->default_handler;
  }

  return handler;
}
