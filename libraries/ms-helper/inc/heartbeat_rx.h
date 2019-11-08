#pragma once
// RX handler for heartbeats.
//
// Requires CAN to be initialized.
//
// Simplifies registering a CanRxHandler such that it automatically replies in
// the affirmative to the message unless the registered callback returns false.
// Register |heartbeat_rx_auto_ack_handler| to always successfully ACK.
//
// The flow control is a follows:
// - A heartbeat master sends a critical CAN message periodically with no data
// expecting certain
//   devices to respond.
// - Each device that is expected to respond should use this module to register
// a handler for that
//   message. Normally this should just involve registering
//   |heartbeat_rx_auto_ack_handler| although in special cases where a device
//   may enter a fault mode a custom handler can be supplied which will
//   intentionally fail to ack if some condition is met.
// - Once the heartbeat master receives all the acks there are two possibilities
//   - The ACK failed: the master will perform some form of fault handling.
//   - The ACK succeeded: the master will send another heartbeat at the next
//   period.

#include <stdbool.h>

#include "can.h"
#include "status.h"

// Return true to respond in the affirmative. Returning false will fail the ACK.
typedef bool (*HeartbeatRxHandler)(CanMessageId msg_id, void *context);

typedef struct HeartbeatRxHandlerStorage {
  HeartbeatRxHandler handler;
  void *context;
} HeartbeatRxHandlerStorage;

// Registers a heartbeat handler (|handler|) to run for |msg_id|. |handler|
// takes |context| as an argument. This configuration is stored in |storage|
// which must persist indefinitely. To automatically respond in the affirmative
// register |heartbeat_rx_auto_ack_handler|.
StatusCode heartbeat_rx_register_handler(HeartbeatRxHandlerStorage *storage, CanMessageId msg_id,
                                         HeartbeatRxHandler handler, void *context);

// An instance of HeartbeatRxHandler that can be used to automatically ack and
// return true with no other behavior.
bool heartbeat_rx_auto_ack_handler(CanMessageId msg_id, void *context);
