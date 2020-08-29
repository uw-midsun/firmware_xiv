#pragma once

// Implementation of sense for reading from MCP3427 ADCs.
// Requires interrupts, soft timers, the event queue, I2C, the data store, sense, and the fault
// handler to be initialized.

#include "data_store.h"
#include "mcp3427_adc.h"

#define MAX_SOLAR_MCP3427 16  // the theoretical maximum on a board

// after this many MCP3427 faults in a row, we raise a fault event
#define MAX_CONSECUTIVE_MCP3427_FAULTS 3

// Configuration for a single MCP3427, associating a data point with the MCP3427 settings.
typedef struct SenseMcp3427AdcConfig {
  Mcp3427Settings mcp3427_settings;
  DataPoint data_point;
  // The factor to multiply raw MCP3427 ADC values by to get the needed units. Results are rounded.
  float scaling_factor;
} SenseMcp3427AdcConfig;

typedef struct SenseMcp3427Settings {
  uint8_t num_mcp3427s;
  // Only the first |num_mcp3427s| entries are used.
  SenseMcp3427AdcConfig mcp3427s[MAX_SOLAR_MCP3427];
} SenseMcp3427Settings;

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_mcp3427_init(SenseMcp3427Settings *settings);

// Start sensing from the MCP3427s. Before this is called, no data will be set in the store.
StatusCode sense_mcp3427_start(void);
