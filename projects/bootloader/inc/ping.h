#pragma once

// This module implements the ping operation on the bootloader
// Should be initialized after dispatcher
#include "status.h"
#include "stdint.h"

// setup the ping response, when a ping is recieved, a response with
// Datagram Id BOOTLOADER_DATAGRAM_PING_RESPONSE(3) will be sent, with data of the board id.
StatusCode ping_init(uint8_t board_id);
