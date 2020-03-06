#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.

#include <stdint.h>
#include "gpio.h"
#include "mcp23008_gpio_expander.h"
#include "mux.h"
#include "status.h"

#define MAX_REAR_POWER_DISTRIBUTION_CURRENTS 32

typedef void (*RearPowerDistributionCurrentMeasurementCallback)(void *context);

typedef struct {
  I2CPort i2c_port;
  uint8_t num_bts7200_channels;
  I2CAddress dsel_i2c_address;                   // I2C address that all the select pins are on
  Mcp23008GpioAddress *bts7200_to_dsel_address;  // map BTS7200 id to select pin address
  MuxAddress mux_address;
  uint8_t *bts7200_to_mux_select;  // map BTS7200 id to SN74 mux select for input
} RearPowerDistributionCurrentHardwareConfig;

typedef struct {
  RearPowerDistributionCurrentHardwareConfig hw_config;
  uint32_t interval_us;
  // If specified, this callback is called whenever current measurements are updated.
  RearPowerDistributionCurrentMeasurementCallback callback;
  void *callback_context;
} RearPowerDistributionCurrentSettings;

typedef struct {
  // Only the first |hw_config->num_bts7200_channels| values will be filled, rest are zeros.
  uint16_t measurements[MAX_REAR_POWER_DISTRIBUTION_CURRENTS];
} RearPowerDistributionCurrentStorage;

// Initialize the module with the settings in the object and set up a soft timer to read currents.
StatusCode rear_power_distribution_current_measurement_init(
    RearPowerDistributionCurrentSettings *settings);

// Return a storage struct containing the latest measurements.
RearPowerDistributionCurrentStorage *rear_power_distribution_current_measurement_get_storage(void);

// Stop periodically reading currents.
StatusCode rear_power_distribution_stop(void);
