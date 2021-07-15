#pragma once

// This module provides a small abstraction over the babydriver CAN message format.
// It allows registering callbacks which will be run when a babydriver CAN message with a specific
// ID is received. It can also automatically respond with a status babydriver CAN message.
// Requires interrupts, gpio, the event queue, and CAN to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "bootloader_datagram_defs.h"
#include "status.h"

// Called whenever a registered bootloader datagram ID is received.
// |data| is the data of the can datagram. This is just a pointer to the data,
//        the pointer should not be stored. As it will be overwrote by the next datagram
// |data_len| is the length of the data.
typedef StatusCode (*DispatcherCallback)(uint8_t *data, uint16_t data_len, void *context);

// Initialize the module.
StatusCode dispatcher_init(void);

// Register |callback| to be called with |context| when a datagram with ID |id| is received.
// Note that there can only be one callback registered per ID at a time, so calling this more than
// once with the same ID will replace the earlier callback.
StatusCode dispatcher_register_callback(BootloaderDatagramId id, DispatcherCallback callback,
                                        void *context);
