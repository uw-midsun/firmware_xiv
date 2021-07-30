#pragma once

// Bootloader Design Confluence: https://uwmidsun.atlassian.net/l/c/76h1GfbH
// This module allows the sending and receiving of CAN messages in the bootloader's CAN ID format.
// It allows registering a callback which will be called when a bootloader CAN message is received
// with client controller board ID 0 (msg_id == 0).
// Requires interrupts, gpio, the event queue, soft timers and CAN to be initialized.
// This modules needs to be initialized before use.

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "status.h"

// Called whenever a bootloader message is received with client controller board ID 0.
typedef StatusCode (*BootloaderCanCallback)(uint8_t *data, size_t len, bool is_start_msg);

// Initializes the module using CanStorage and CanSettings, without needing to specify device_id
// device_id will be overwritten to be SYSTEM_CAN_DEVICE_BOOTLOADER.
// any CAN message transmitted with this module will have board_id as its msg_id
StatusCode bootloader_can_init(CanStorage *storage, CanSettings *settings, uint16_t board_id);

// Transmits a bootloader datagram message with the given data of length len <=8
// it being a start message if is_start_message is true and a non-start message otherwise.
StatusCode bootloader_can_transmit(uint8_t *data, size_t len, bool is_start_message);

// Registers a handler to be called when a bootloader CAN message is received,
// only one handler can be registered at a time.
StatusCode bootloader_can_register_handler(BootloaderCanCallback callback);

// Register a handler to be called when a datagram message is recieved
// only to be used for testing
StatusCode bootloader_can_register_debug_handler(BootloaderCanCallback callback);

// This function is mainly for internal use in the CAN library, does not have to be called by user.
// It calls the registered handler function of the message if the msg_id is equal to the
// client ID, 0. Otherwise, the message is ignored.
StatusCode bootloader_can_receive(CanMessage *msg);
