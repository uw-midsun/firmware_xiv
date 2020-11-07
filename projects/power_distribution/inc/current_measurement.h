#pragma once

// Periodically reads current from load switches and exposes global storage.
// Requires GPIO, interrupts, soft timers, ADC (in ADC_MODE_SINGLE), and I2C to be initalized.

#include <stdint.h>

#include "bts7xxx_common.h"
#include "currents.h"
#include "gpio.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"
#include "status.h"

#define MAX_POWER_DISTRIBUTION_BTS7200_CHANNELS 16  // max BTS7200s per board
#define MAX_POWER_DISTRIBUTION_BTS7040_CHANNELS 16  // max BTS7040s per board

// Experimentally determined, more accurate resistor value to convert BTS7200 sensed current
// (note that there are 1.6k resistors on the current rev as of 2020-10-31, but the scaling factor
// was experimentally determined to imply this resistor value)
#define POWER_DISTRIBUTION_BTS7200_SENSE_RESISTOR 1160

// Experimentally determined bias in the BTS7200 sensed output
#define POWER_DISTRIBUTION_BTS7200_BIAS (-8)

// All BTS7040s use a 1.21k resistor to convert sense current
#define POWER_DISTRIBUTION_BTS7040_SENSE_RESISTOR 1210

// To be calibrated - bias in the BTS7040 sensed output
#define POWER_DISTRIBUTION_BTS7040_BIAS 0

// Voltage at the SENSE pin is limited to a max of 3.3V by a diode.
// Due to to this function, since any fault current will be at least 4.4 mA (see p.g. 49)
// the resulting voltage will be 4.4 mA * 1.6 kOhm = ~7 V. Due to this,
// voltages approaching 3.3V represent a fault, and should be treated as such.
// Max doesn't matter much, so it's left as a high value to account for any errors.
#define POWER_DISTRIBUTION_BTS7200_MIN_FAULT_VOLTAGE_MV 3200
#define POWER_DISTRIBUTION_BTS7200_MAX_FAULT_VOLTAGE_MV 10000

// Apply similar logic for the BTS7040s (see p.g. 49 of BTS7040 datasheet):
#define POWER_DISTRIBUTION_BTS7040_MIN_FAULT_VOLTAGE_MV 3200
#define POWER_DISTRIBUTION_BTS7040_MAX_FAULT_VOLTAGE_MV 10000

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
  Pca9539rGpioAddress en_pin;
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
  GpioAddress mux_output_pin;
  GpioAddress mux_enable_pin;
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

// TODO(SOFT-336): replace this with a less janky system that doesn't involve double pointers.
// Return an array of pointers to the enable pins of the BTS7200s/7040s in no particular order.
Bts7xxxEnablePin **power_distribution_current_measurement_get_pins(size_t *num_pins);
