#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC, and I2C to be initalized.

#include "status.h"

typedef enum {
  FRONT_POWER_DISTRIBUTION_CURRENT_1 = 0, // not sure what they are, change this later
  FRONT_POWER_DISTRIBUTION_CURRENT_2,
  FRONT_POWER_DISTRIBUTION_CURRENT_3,
  FRONT_POWER_DISTRIBUTION_CURRENT_4,
  FRONT_POWER_DISTRIBUTION_CURRENT_5,
  FRONT_POWER_DISTRIBUTION_CURRENT_6,
  FRONT_POWER_DISTRIBUTION_CURRENT_7,
  FRONT_POWER_DISTRIBUTION_CURRENT_8,
  NUM_FRONT_POWER_DISTRIBUTION_CURRENTS,
} FrontPowerDistributionCurrentMeasurement;

typedef struct {
  uint16_t measurements[NUM_FRONT_POWER_DISTRIBUTION_CURRENTS];
} FrontPowerDistributionCurrentStorage;

StatusCode front_power_distribution_current_measurement_init(void);

StatusCode front_power_distribution_current_measurement_get_storage(FrontPowerDistributionCurrentStorage *storage);
