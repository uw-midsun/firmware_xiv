#pragma once

// Bootloader Design Confluence: https://uwmidsun.atlassian.net/l/c/76h1GfbH
// This module allows the sending and receiving of CAN messages in the bootloader's CAN ID format.
// It allows registering a callback which will be called when a bootloader CAN message is received
// with client controller board ID 0 (msg_id == 0).
// Requires interrupts, gpio, the event queue, soft timers and CAN to be initialized.
// If only bootloader datagrams are to be transmitted, the bootloader module needs to be initialized
// otherwise, initializing CAN is sufficient.

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "status.h"

// Called whenever a bootloader message is received with client controller board ID 0.
typedef StatusCode (*BootloaderCanCallback)(const CanMessage *msg, void *context);

// This function only needs to be called to transmit bootloader datagrams only,
// otherwise calling can_init is valid.
// Initializes the module using CanStorage and CanSettings, without needing to specify device_id
// device_id will be overwritten to be SYSTEM_CAN_DEVICE_BOOTLOADER.
StatusCode bootloader_can_init(CanStorage *storage, CanSettings *settings);

// Transmits a bootloader datagram message with the given data of length len <=8, from the given
// board id, it being a start message if is_start_message is true and a non-start message otherwise.
StatusCode bootloader_can_transmit(uint16_t board_id, uint8_t *data, size_t len,
                                   bool is_start_message);

// Registers a handler to be called when a bootloader CAN message is received,
// only one handler can be registered at a time.
StatusCode bootloader_can_register_handler(BootloaderCanCallback callback, void *context);

// This function is mainly for internal use in the CAN library, does not have to be called by user.
// It calls the registered handler function of the message if the msg_id is equal to the
// client ID, 0. Otherwise, the message is ignored.
StatusCode bootloader_can_receive(CanMessage *msg);
