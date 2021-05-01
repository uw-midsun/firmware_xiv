#pragma once

// This module allows the sending and receiving of CAN messages in the bootloader's CAN ID format.
// It allows registering a callback which will be called when a bootloader CAN message is received
// with client controller board ID 0 (msg_id == 0).

#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "status.h"


// Initialize the module using CanStorage and CanSettings, without needing to specify device_id
// device_id will be overwritten to be SYSTEM_CAN_DEVICE_BOOTLOADER
StatusCode bootloader_can_init(CanStorage *storage, CanSettings *settings);

// Transmits a CAN message with data of length len, and sets the message type = CAN_MSG_TYPE_ACK 
// if is_start_message is true, otherwise, type = CAN_MSG_TYPE_DATA
StatusCode bootloader_can_transmit(uint8_t *data, size_t len, bool is_start_message, uint16_t board_id);

// Stores the callback and context for later use, if callback or context is NULL,
// the function will return STATUS_CODE_INVALID_ARGS
StatusCode bootloader_can_register_handler(CanRxHandlerCb callback, void *context);

// Calls the registered handler function of the message if the msg_id is equal to the client ID, 0
// Otherwise, the message is ignored
StatusCode bootloader_can_receive(CanMessage *msg);
