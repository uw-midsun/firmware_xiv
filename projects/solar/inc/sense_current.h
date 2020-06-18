#pragma once

// Implementation of sense for reading from the current-sense MCP3427.
// Requires interrupts, soft timers, the event queue, I2C, and sense to be initialized.

#include "status.h"

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_current_init(void);
