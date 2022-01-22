#pragma once

// This module implements the update ID operation on the bootloader
// Should be initialized after dispatcher
#include "status.h"
#include "stdint.h"

// setup the update ID response, when a datagram containing datagram ID:
// BOOTLOADER_DATAGRAM_UPDATE_ID is recieved, a response with datagram ID
// BOOTLOADER_DATAGRAM_STATUS_RESPONSE will be sent, with the status code as data. Additionaly the
// ID will be updated in config and the board will be reset after the datagram is sent
StatusCode update_id_init(void);
