#pragma once

// Standard hardware configurations for current_measurement.

#include <stdint.h>

#include "gpio.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"

typedef struct {
  Pca9539rGpioAddress dsel_pin;
  Pca9539rGpioAddress en0_pin;
  Pca9539rGpioAddress en1_pin;
  uint8_t mux_selection;
} PowerDistributionBts7200Data;

typedef struct {
  I2CPort i2c_port;

  // All distinct I2C addresses on which there are BTS7200 DSEL pins
  I2CAddress *dsel_i2c_addresses;
  uint8_t num_dsel_i2c_addresses;  // length of preceding array

  // Data of all BTS7200s from which to read
  PowerDistributionBts7200Data *bts7200s;
  uint8_t num_bts7200_channels;  // length of preceding array

  MuxAddress mux_address;
  GpioAddress mux_output_pin;
  GpioAddress mux_enable_pin;
} PowerDistributionCurrentHardwareConfig;

extern const PowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_CURRENT_config;
extern const PowerDistributionCurrentHardwareConfig REAR_POWER_DISTRIBUTION_CURRENT_config;
