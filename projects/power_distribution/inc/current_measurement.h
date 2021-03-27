#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC, I2C, and output to be initalized.

#include <stdint.h>

#include "output.h"
#include "status.h"

typedef void (*PowerDistributionCurrentMeasurementCallback)(void *context);

typedef struct {
  // An array of all outputs from which to read.
  Output *outputs_to_read;
  uint8_t num_outputs_to_read; // length of preceding array
} PowerDistributionCurrentHardwareConfig;

typedef struct {
  PowerDistributionCurrentHardwareConfig *hw_config;
  uint32_t interval_us;
  // If specified, this callback is called whenever current measurements are updated.
  PowerDistributionCurrentMeasurementCallback callback;
  void *callback_context;
} PowerDistributionCurrentSettings;

typedef struct {
  // Only the outputs specified in the hardware config will be populated.
  uint16_t measurements[NUM_OUTPUTS];
} PowerDistributionCurrentStorage;

// Initialize the module with the given settings and set up a soft timer to read currents.
StatusCode power_distribution_current_measurement_init(PowerDistributionCurrentSettings *settings);

// Return a storage struct containing the latest measurements.
PowerDistributionCurrentStorage *power_distribution_current_measurement_get_storage(void);

// Stop periodically reading currents.
StatusCode power_distribution_current_measurement_stop(void);
