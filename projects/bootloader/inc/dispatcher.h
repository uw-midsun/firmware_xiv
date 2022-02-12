#pragma once

// This module provides a small abstraction over the bootloader Can Datagram format.
// It allows registering callbacks which will be run when a Datagram with a specific ID is received.
// Requires interrupts, gpio, the event queue, bootloader CAN, and CAN datagram to be initialized.

#include <stdint.h>

#include "bootloader_datagram_defs.h"
#include "can_datagram.h"
#include "status.h"
#include "stdbool.h"

// Called whenever a registered bootloader datagram ID is received.
// |data| is the data of the can datagram. This is just a pointer to the data,
//        the pointer should not be stored. As it will be overwrote by the next datagram
// |data_len| is the length of the data.
typedef StatusCode (*DispatcherCallback)(uint8_t *data, uint16_t data_len, void *context);

// Initialize the module.
StatusCode dispatcher_init(uint8_t board_id);

// Register |callback| to be called with |context| when a datagram with ID |id| is received.
// Note that there can only be one callback registered per ID at a time, so calling this more than
// once with the same ID will replace the earlier callback.
StatusCode dispatcher_register_callback(BootloaderDatagramId id, DispatcherCallback callback,
                                        void *context);

// this should be used as the tx_cmpl_cb in every tx datagram
// this function should not be called directly
void tx_cmpl_cb(void);

// This function returns a status code in a datagram
// used by any bootloader operation that respond with a status code
// |callback| is called after the datagram completes transmission.
// It also allows a callback function to be set, which will
// be triggered at the end of the tx
StatusCode status_response(StatusCode code, CanDatagramExitCb callback);
