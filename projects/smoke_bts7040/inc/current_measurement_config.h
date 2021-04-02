#pragma once

// Standard hardware configurations for current_measurement.

#include <stdint.h>

#include "gpio.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"

typedef struct {
  Pca9539rGpioAddress en_pin;
  uint8_t mux_selection;
} PowerDistributionBts7040Data;

typedef struct {
  I2CPort i2c_port;

  // All distinct I2C addresses on which there are BTS7200 DSEL pins
  I2CAddress *dsel_i2c_addresses;
  uint8_t num_dsel_i2c_addresses;  // length of preceding array

  // Data of all BTS7040/7008s from which to read
  PowerDistributionBts7040Data *bts7040s;
  uint8_t num_bts7040_channels;  // length of preceding array

  MuxAddress mux_address;
  GpioAddress mux_output_pin;
  GpioAddress mux_enable_pin;
} PowerDistributionCurrentHardwareConfig;

extern const PowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;
extern const PowerDistributionCurrentHardwareConfig REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG;
