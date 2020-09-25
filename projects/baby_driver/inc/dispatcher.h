#pragma once

// This module provides a small abstraction over the babydriver CAN message format.
// It allows registering callbacks which will be run when a babydriver CAN message with a specific
// ID is received. It can also automatically respond with a status babydriver CAN message.
// Requires interrupts, gpio, the event queue, and CAN to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "babydriver_msg_defs.h"
#include "status.h"

// Called whenever a registered babydriver message ID is received.
// |data| is the 8 bytes of data from the message (data[0] will be the registered ID).
// |*tx_result| is true by default. If it is true when the callback returns, dispatcher will TX
// a status babydriver CAN message with the returned StatusCode. Set it to false to suppress this.
typedef StatusCode (*DispatcherCallback)(uint8_t data[8], void *context, bool *tx_result);

// Initialize the module.
StatusCode dispatcher_init(void);

// Register |callback| to be called with |context| when a message with ID |id| is received.
// Note that there can only be one callback registered per ID at a time, so calling this more than
// once with the same ID will replace the earlier callback.
StatusCode dispatcher_register_callback(BabydriverMessageId id, DispatcherCallback callback,
                                        void *context);
