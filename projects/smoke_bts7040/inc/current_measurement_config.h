#pragma once

// Standard hardware configurations for current_measurement.

// #include "current_measurement.h"
#include <stdint.h>
#include "gpio.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"

typedef struct {
  uint8_t mux_selection;
} PowerDistributionBts7040Data;

typedef struct {
  // Data of all BTS7040/7008s from which to read
  PowerDistributionBts7040Data *bts7040s;
  uint8_t num_bts7040_channels;  // length of preceding array

  MuxAddress mux_address;
} PowerDistributionCurrentHardwareConfig;

extern const PowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;
extern const PowerDistributionCurrentHardwareConfig REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;
