#pragma once

// This module impliments the ping operation on the bootloader
#include "status.h"

// send a datagram response to a ping message
// datagram will have an ID of BOOTLOADER_DATAGRAM_PING_RESPONSE(3), and data contains the board id.
StatusCode ping_response(void);
