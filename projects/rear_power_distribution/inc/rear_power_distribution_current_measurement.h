#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.
// I2C must be initialized on I2C_PORT_2 with the SDA and SCL addresses below.

#include <stdint.h>
#include "gpio.h"
#include "status.h"

extern const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SDA_ADDRESS;

extern const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SCL_ADDRESS;

typedef enum {
  REAR_POWER_DISTRIBUTION_CURRENT_CTR_BRK_LIGHT = 0,
  REAR_POWER_DISTRIBUTION_CURRENT_STROBE,
  REAR_POWER_DISTRIBUTION_CURRENT_LEFT_BRK_LIGHT,
  REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_BRK_LIGHT,
  REAR_POWER_DISTRIBUTION_CURRENT_LEFT_TURN,
  REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_TURN,
  REAR_POWER_DISTRIBUTION_CURRENT_SOLAR,
  REAR_POWER_DISTRIBUTION_CURRENT_TELEMETRY,
  NUM_REAR_POWER_DISTRIBUTION_CURRENTS,
} RearPowerDistributionCurrentMeasurement;

typedef void (*RearPowerDistributionCurrentCallback)(void *context);

typedef struct {
  uint32_t interval_us;
  // If specified, this callback is called whenever current measurements are updated.
  RearPowerDistributionCurrentCallback callback;
  void *callback_context;
} RearPowerDistributionCurrentSettings;

typedef struct {
  uint16_t measurements[NUM_REAR_POWER_DISTRIBUTION_CURRENTS];
} RearPowerDistributionCurrentStorage;

// Initialize the module with the given settings and set up a soft timer to read currents.
StatusCode rear_power_distribution_current_measurement_init(
    RearPowerDistributionCurrentSettings *settings);

// Copy the latest values to the given storage.
StatusCode rear_power_distribution_current_measurement_get_storage(
    RearPowerDistributionCurrentStorage *storage);

// Stop periodically reading currents.
StatusCode rear_power_distribution_stop(void);
