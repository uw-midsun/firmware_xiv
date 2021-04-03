#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC, I2C, and output to be initalized.

#include <stdint.h>

#include "output.h"
#include "status.h"

typedef void (*CurrentMeasurementCallback)(void *context);

typedef struct {
  // An array of all outputs from which to read.
  Output *outputs_to_read;
  uint8_t num_outputs_to_read;  // length of preceding array
} CurrentMeasurementConfig;

typedef struct {
  CurrentMeasurementConfig *config;
  uint32_t interval_us;
  // If specified, this callback is called whenever current measurements are updated.
  CurrentMeasurementCallback callback;
  void *callback_context;
} CurrentMeasurementSettings;

typedef struct {
  // Only the outputs specified in the hardware config will be populated.
  uint16_t measurements[NUM_OUTPUTS];
} CurrentMeasurementStorage;

// Initialize the module with the given settings and set up a soft timer to read currents.
StatusCode current_measurement_init(CurrentMeasurementSettings *settings);

// Return a storage struct containing the latest measurements in mA.
CurrentMeasurementStorage *current_measurement_get_storage(void);

// Stop periodically reading currents.
StatusCode current_measurement_stop(void);
