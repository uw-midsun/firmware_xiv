#pragma once

// This module registers a callback with the dispatcher
// for jump_to_application operation on the bootloader
#include "status.h"
#include "stdint.h"

// This function registers a dispatcher callback
// and intializes the jump to application
// if the application_crc32 code of config matches the computed
// crc32 of the flash memory
StatusCode jump_to_application_dispatcher_init();
