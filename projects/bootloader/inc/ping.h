#pragma once

// This module impliments the ping operation on the bootloader
#include "status.h"

// setup the ping response, when a ping is recieved, a response with
// Datagram Id BOOTLOADER_DATAGRAM_PING_RESPONSE(3) will be sent, with data of the board id.
StatusCode ping_init(void);
