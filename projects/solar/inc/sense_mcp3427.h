#pragma once

// Implementation of sense for reading from MCP3427 ADCs.
// Requires interrupts, soft timers, the event queue, I2C, data_store, and sense to be initialized.

#include "data_store.h"
#include "mcp3427_adc.h"

#define MAX_SOLAR_MCP3427 8

// Configuration for a single MCP3427, associating a data point with the MCP3427 settings.
typedef struct SenseMcp3427AdcConfig {
  Mcp3427Settings mcp3427_settings;
  DataPoint data_point;
} SenseMcp3427AdcConfig;

typedef struct SenseMcp3427Settings {
  uint8_t num_mcp3427s;
  // Only the first |num_mcp3427s| entries are used.
  SenseMcp3427AdcConfig mcp3427s[MAX_SOLAR_MCP3427];
} SenseMcp3427Settings;

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_mcp3427_init(SenseMcp3427Settings *settings);
