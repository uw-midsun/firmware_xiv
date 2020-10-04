#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.

#include <stdint.h>
#include "currents.h"
#include "gpio.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"
#include "status.h"

#define MAX_POWER_DISTRIBUTION_BTS7200_CHANNELS 16  // max BTS7200s per board
#define MAX_POWER_DISTRIBUTION_BTS7040_CHANNELS 16  // max BTS7040s per board

typedef void (*PowerDistributionCurrentMeasurementCallback)(void *context);

typedef struct {
  Pca9539rGpioAddress dsel_pin;
  Pca9539rGpioAddress en0_pin;
  Pca9539rGpioAddress en1_pin;
  PowerDistributionCurrent current_0;
  PowerDistributionCurrent current_1;
  uint8_t mux_selection;
} PowerDistributionBts7200Data;

typedef struct {
  PowerDistributionCurrent current;
  uint8_t mux_selection;
} PowerDistributionBts7040Data;

typedef struct {
  I2CPort i2c_port;

  // All distinct I2C addresses on which there are BTS7200 DSEL pins
  I2CAddress *dsel_i2c_addresses;
  uint8_t num_dsel_i2c_addresses;  // length of preceding array

  // Data of all BTS7200s from which to read
  PowerDistributionBts7200Data *bts7200s;
  uint8_t num_bts7200_channels;  // length of preceding array

  // Data of all BTS7040/7008s from which to read
  PowerDistributionBts7040Data *bts7040s;
  uint8_t num_bts7040_channels;  // length of preceding array

  MuxAddress mux_address;
} PowerDistributionCurrentHardwareConfig;

typedef struct {
  PowerDistributionCurrentHardwareConfig hw_config;
  uint32_t interval_us;
  // If specified, this callback is called whenever current measurements are updated.
  PowerDistributionCurrentMeasurementCallback callback;
  void *callback_context;
} PowerDistributionCurrentSettings;

typedef struct {
  // Only the currents specified in the hardware config will be populated.
  uint16_t measurements[NUM_POWER_DISTRIBUTION_CURRENTS];
} PowerDistributionCurrentStorage;

// Initialize the module with the given settings and set up a soft timer to read currents.
StatusCode power_distribution_current_measurement_init(PowerDistributionCurrentSettings *settings);

// Return a storage struct containing the latest measurements.
PowerDistributionCurrentStorage *power_distribution_current_measurement_get_storage(void);

// Stop periodically reading currents.
StatusCode power_distribution_current_measurement_stop(void);
