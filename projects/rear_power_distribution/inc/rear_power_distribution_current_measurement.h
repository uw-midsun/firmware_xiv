#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.

#include <stdint.h>
#include "gpio.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"
#include "status.h"

#define MAX_REAR_POWER_DISTRIBUTION_BTS7200_CHANNELS 16  // max BTS7200s per board
#define MAX_REAR_POWER_DISTRIBUTION_BTS7040_CHANNELS 16  // max BTS7040s per board

typedef void (*RearPowerDistributionCurrentMeasurementCallback)(void *context);

typedef enum {
  REAR_POWER_DISTRIBUTION_CURRENT_LEFT_CAMERA = 0,
  REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_CAMERA,
  REAR_POWER_DISTRIBUTION_CURRENT_MAIN_DISPLAY,
  REAR_POWER_DISTRIBUTION_CURRENT_REAR_DISPLAY,
  REAR_POWER_DISTRIBUTION_CURRENT_DRIVER_DISPLAY,
  REAR_POWER_DISTRIBUTION_CURRENT_LEFT_DRIVER_DISPLAY,
  REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_DRIVER_DISPLAY,
  REAR_POWER_DISTRIBUTION_CURRENT_MAIN_PI,
  REAR_POWER_DISTRIBUTION_CURRENT_REAR_PI,
  REAR_POWER_DISTRIBUTION_CURRENT_LEFT_FRONT_TURN_LIGHT,
  REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_FRONT_TURN_LIGHT,
  REAR_POWER_DISTRIBUTION_CURRENT_DAYTIME_RUNNING_LIGHTS,
  REAR_POWER_DISTRIBUTION_CURRENT_CENTRE_CONSOLE,
  REAR_POWER_DISTRIBUTION_CURRENT_PEDAL,
  REAR_POWER_DISTRIBUTION_CURRENT_STEERING,
  REAR_POWER_DISTRIBUTION_CURRENT_PARKING_BRAKE,
  REAR_POWER_DISTRIBUTION_CURRENT_SPEAKER,
  REAR_POWER_DISTRIBUTION_CURRENT_HORN,
  REAR_POWER_DISTRIBUTION_CURRENT_5V_SPARE_1,
  REAR_POWER_DISTRIBUTION_CURRENT_5V_SPARE_2,
  REAR_POWER_DISTRIBUTION_CURRENT_SPARE_1,
  REAR_POWER_DISTRIBUTION_CURRENT_SPARE_2,
  NUM_REAR_POWER_DISTRIBUTION_CURRENTS,
} RearPowerDistributionCurrent;

typedef struct {
  Pca9539rGpioAddress dsel_gpio_address;
  Pca9539rGpioAddress enable_pin_0;
  Pca9539rGpioAddress enable_pin_1;
  RearPowerDistributionCurrent current_0;
  RearPowerDistributionCurrent current_1;
  uint8_t mux_selection;
} RearPowerDistributionBts7200Data;

typedef struct {
  Pca9539rGpioAddress enable_gpio_address;
  RearPowerDistributionCurrent current;
  uint8_t mux_selection;
} RearPowerDistributionBts7040Data;

typedef struct {
  I2CPort i2c_port;

  // All distinct I2C addresses on which there are BTS7200 DSEL pins
  I2CAddress *dsel_i2c_addresses;
  uint8_t num_dsel_i2c_addresses;  // length of preceding array

  // Data of all BTS7200s from which to read
  RearPowerDistributionBts7200Data *bts7200s;
  uint8_t num_bts7200_channels;  // length of preceding array

  // Data of all BTS7040/7008s from which to read
  RearPowerDistributionBts7040Data *bts7040s;
  uint8_t num_bts7040_channels;  // length of preceding array

  MuxAddress mux_address;
} RearPowerDistributionCurrentHardwareConfig;

typedef struct {
  RearPowerDistributionCurrentHardwareConfig hw_config;
  uint32_t interval_us;
  // If specified, this callback is called whenever current measurements are updated.
  RearPowerDistributionCurrentMeasurementCallback callback;
  void *callback_context;
} RearPowerDistributionCurrentSettings;

typedef struct {
  // Only the currents specified in the hardware config will be populated.
  uint16_t measurements[NUM_REAR_POWER_DISTRIBUTION_CURRENTS];
} RearPowerDistributionCurrentStorage;

// Initialize the module with the settings in the object and set up a soft timer to read currents.
StatusCode rear_power_distribution_current_measurement_init(
    RearPowerDistributionCurrentSettings *settings);

// Return a storage struct containing the latest measurements.
RearPowerDistributionCurrentStorage *rear_power_distribution_current_measurement_get_storage(void);

// Stop periodically reading currents.
StatusCode rear_power_distribution_stop(void);
