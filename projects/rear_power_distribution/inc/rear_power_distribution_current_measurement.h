#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.
// I2C must be initialized on I2C_PORT_2 with the SDA and SCL addresses below.

#include <stdint.h>
#include "gpio.h"
#include "mcp23008_gpio_expander.h"
#include "sn74_mux.h"
#include "status.h"

#define MAX_REAR_POWER_DISTRIBUTION_CURRENTS 32

extern const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SDA_ADDRESS;
extern const GpioAddress REAR_POWER_DISTRIBUTION_CURRENT_I2C_SCL_ADDRESS;

typedef void (*RearPowerDistributionCurrentMeasurementCallback)(void *context);

typedef struct {
  uint8_t num_bts7200_channels;
  Mcp23008I2CAddress dsel_i2c_address;           // I2C address that all the select pins are on
  Mcp23008GpioAddress *bts7200_to_dsel_address;  // map BTS7200 id to select pin address
  Sn74MuxAddress mux_address;
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
