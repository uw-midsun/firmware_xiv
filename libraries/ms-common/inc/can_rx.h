#pragma once
// CAN RX handlers
// Provides an interface for registering and finding callbacks based on CAN
// message IDs.
#include <stdint.h>
#include "can_ack.h"
#include "can_msg.h"
#include "status.h"

// Process the received CAN message. Storage to an ACK reply is valid only if
// the message is considered critical. If valid, the ack reply is OK by default.
typedef StatusCode (*CanRxHandlerCb)(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

typedef struct CanRxHandler {
  CanRxHandlerCb callback;
  void *context;
  CanMessageId msg_id;
} CanRxHandler;

typedef struct CanRxHandlers {
  CanRxHandler *storage;
  CanRxHandler *default_handler;
  size_t max_handlers;
  size_t num_handlers;
} CanRxHandlers;

StatusCode can_rx_init(CanRxHandlers *rx_handlers, CanRxHandler *handler_storage,
                       size_t num_handlers);

StatusCode can_rx_register_default_handler(CanRxHandlers *rx_handlers, CanRxHandlerCb handler,
                                           void *context);

StatusCode can_rx_register_handler(CanRxHandlers *rx_handlers, CanMessageId msg_id,
                                   CanRxHandlerCb handler, void *context);

CanRxHandler *can_rx_get_handler(CanRxHandlers *rx_handlers, CanMessageId msg_id);
