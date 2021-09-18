#pragma once

// This module implements the query operation on the bootloader
// Should be initialized after dispatcher

#include <stdint.h>

#include "status.h"

// forward declaration
typedef struct BootloaderConfig BootloaderConfig;

// setup the query operation, when a query with pattern-matching fields is received
// send a response datagram with the full board config information back.
StatusCode query_init(BootloaderConfig *config);
