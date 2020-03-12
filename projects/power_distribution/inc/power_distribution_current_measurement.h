#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.

#include <stdint.h>
#include "gpio.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"
#include "status.h"

#define MAX_POWER_DISTRIBUTION_BTS7200_CHANNELS 16  // max BTS7200s per board
#define MAX_POWER_DISTRIBUTION_BTS7040_CHANNELS 16  // max BTS7040s per board

typedef void (*PowerDistributionCurrentMeasurementCallback)(void *context);

typedef enum {
  // Currents for front power distribution
  POWER_DISTRIBUTION_CURRENT_LEFT_CAMERA = 0,
  POWER_DISTRIBUTION_CURRENT_RIGHT_CAMERA,
  POWER_DISTRIBUTION_CURRENT_MAIN_DISPLAY,
  POWER_DISTRIBUTION_CURRENT_REAR_DISPLAY,
  POWER_DISTRIBUTION_CURRENT_DRIVER_DISPLAY,
  POWER_DISTRIBUTION_CURRENT_LEFT_DRIVER_DISPLAY,
  POWER_DISTRIBUTION_CURRENT_RIGHT_DRIVER_DISPLAY,
  POWER_DISTRIBUTION_CURRENT_MAIN_PI,
  POWER_DISTRIBUTION_CURRENT_REAR_PI,
  POWER_DISTRIBUTION_CURRENT_LEFT_FRONT_TURN_LIGHT,
  POWER_DISTRIBUTION_CURRENT_RIGHT_FRONT_TURN_LIGHT,
  POWER_DISTRIBUTION_CURRENT_DAYTIME_RUNNING_LIGHTS,
  POWER_DISTRIBUTION_CURRENT_CENTRE_CONSOLE,
  POWER_DISTRIBUTION_CURRENT_PEDAL,
  POWER_DISTRIBUTION_CURRENT_STEERING,
  POWER_DISTRIBUTION_CURRENT_PARKING_BRAKE,
  POWER_DISTRIBUTION_CURRENT_SPEAKER,
  POWER_DISTRIBUTION_CURRENT_HORN,
  POWER_DISTRIBUTION_CURRENT_5V_SPARE_1,
  POWER_DISTRIBUTION_CURRENT_5V_SPARE_2,
  POWER_DISTRIBUTION_CURRENT_SPARE_1,
  POWER_DISTRIBUTION_CURRENT_SPARE_2,

  // Currents for rear power distribution
  // are these accurate?
  POWER_DISTRIBUTION_CURRENT_REAR_BRAKE_LIGHT,
  POWER_DISTRIBUTION_CURRENT_LEFT_REAR_TURN_LIGHT,
  POWER_DISTRIBUTION_CURRENT_RIGHT_REAR_TURN_LIGHT,
  POWER_DISTRIBUTION_CURRENT_REAR_CAMERA,
  POWER_DISTRIBUTION_CURRENT_PLACEHOLDER_REAR_CAM,  // rear cam's BTS7200 partner, unconnected (??)
  POWER_DISTRIBUTION_CURRENT_CHARGER_INTERFACE,
  POWER_DISTRIBUTION_CURRENT_LIGHTS_BPS_STROBE,
  POWER_DISTRIBUTION_CURRENT_BMS_CARRIER,
  POWER_DISTRIBUTION_CURRENT_MCI,
  POWER_DISTRIBUTION_CURRENT_SOLAR_SENSE,

  NUM_POWER_DISTRIBUTION_CURRENTS,
} PowerDistributionCurrent;

typedef struct {
  Pca9539rGpioAddress dsel_pin;
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
StatusCode power_distribution_stop(void);
