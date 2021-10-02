#pragma once

// This module implements the ping operation on the bootloader
// Should be initialized after dispatcher

#include <stdint.h>

#include "status.h"

// setup the ping response, when a ping is recieved, a response with
// Datagram Id BOOTLOADER_DATAGRAM_PING_RESPONSE will be sent, with the |board_id| as data.
StatusCode ping_init(uint8_t board_id);
