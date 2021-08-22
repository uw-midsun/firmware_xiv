#pragma once

// Provides an interface for hooking into CAN.
// This is a separate module to keep the main can module's interface clean.
// Requires CAN and its dependencies to be initialized.

#include "can.h"
#include "can_msg.h"
#include "status.h"

typedef void (*CanTxHook)(const CanMessage *msg, void *context);

// Initialize the hook module with respect to the given CAN storage.
void can_hook_init(CanStorage *storage);

// Register a hook.
// |hook| will be called whenever a CAN TX is attempted, i.e. on every can_transmit call.
// (Note: this is not the same as an actual TX. The TX might be delayed if the queue is non-empty.)
StatusCode can_hook_tx_register(CanTxHook hook, void *context);

// Cause a CAN message to be RXed. Note that this may fail if the CAN fifo or event queue is full.
StatusCode can_hook_rx(const CanMessage *msg);

// Called when a CAN TX is attempted. For internal and test use.
void can_hook_tx_trigger(const CanMessage *msg);
