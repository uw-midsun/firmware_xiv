#pragma once

// This module implements the query operation on the bootloader
// Should be initialized after dispatcher

#include <stdint.h>

#include "status.h"

// setup the query operation, when a query with pattern-matching fields is received
// send a response datagram with the full board config information back.
StatusCode query_init(uint8_t id, char *name, char *current_project, char *project_info,
                      char *git_version);
