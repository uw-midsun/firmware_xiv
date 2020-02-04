#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.

#include <stdint.h>
#include "status.h"

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

typedef struct {
  uint16_t measurements[NUM_REAR_POWER_DISTRIBUTION_CURRENTS];
} RearPowerDistributionCurrentStorage;

StatusCode rear_power_distribution_current_measurement_init(void);

StatusCode rear_power_distribution_current_measurement_get_storage(RearPowerDistributionCurrentStorage *storage);
