#pragma once

// Implementation of sense for reading from the current-sense MCP3427.
// Requires interrupts, soft timers, the event queue, I2C, and sense to be initialized.

#include "mcp3427_adc.h"

typedef struct SenseCurrentSettings {
  Mcp3427Settings *current_mcp3427_settings;
} SenseCurrentSettings;

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_current_init(SenseCurrentSettings *settings);
