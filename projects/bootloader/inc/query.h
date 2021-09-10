#pragma once

// This module implements the query operation on the bootloader
// Should be initialized after dispatcher

#include "config.h"
#include "status.h"

// setup the query operation, when a query with pattern-matching fields is received
// send a response datagram with the full board config information back.
StatusCode query_init(BootloaderConfig *config);
