#pragma once

// This module registers a callback with the dispatcher
// for jump_to_application operation on the bootloader
#include "status.h"
#include "stdint.h"

#define FAILURE_STATUS 1
#define SUCCESS_STATUS 0

// setup the ping response, when a ping is recieved, a response with
// Datagram Id BOOTLOADER_DATAGRAM_PING_RESPONSE will be sent, with the |board_id| as data.
StatusCode jump_to_application_dispatcher_init();
