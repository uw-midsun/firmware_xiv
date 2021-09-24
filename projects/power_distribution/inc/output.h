#pragma once

// General-purpose module for manipulating the outputs that power distribution controls.
// Requires GPIO, interrupts, soft timers, ADC, and I2C to be initialized.
//
// An output is an abstraction of "something that PD can turn on and off".
// This module provides a uniform interface for manipulating outputs implemented through GPIO or
// an IO expander, and through a BTS7200 or BTS7040 load switch or not.

#include <stdbool.h>
#include <stdint.h>

#include "bts7200_load_switch.h"
#include "gpio.h"
#include "i2c.h"
#include "mux.h"
#include "pca9539r_gpio_expander.h"
#include "status.h"

// Please don't change the numerical values of the outputs so downstream tools can rely on them.
// Add any new outputs at the end, and if a spare gets a proper name, just rename the spare output.
typedef enum {
  // Outputs for front power distribution
  FRONT_OUTPUT_CENTRE_CONSOLE = 0,
  FRONT_OUTPUT_PEDAL,
  FRONT_OUTPUT_STEERING,
  FRONT_OUTPUT_DRIVER_DISPLAY,
  FRONT_OUTPUT_INFOTAINMENT_DISPLAY,  // aka main display
  FRONT_OUTPUT_LEFT_DISPLAY,
  FRONT_OUTPUT_RIGHT_DISPLAY,
  FRONT_OUTPUT_REAR_DISPLAY,
  FRONT_OUTPUT_LEFT_CAMERA,
  FRONT_OUTPUT_RIGHT_CAMERA,
  FRONT_OUTPUT_MAIN_PI,  // driver display + telemetry pi
  FRONT_OUTPUT_SPEAKER,
  FRONT_OUTPUT_FAN,  // on UV cutoff (not on load switch), no current sense
  FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
  FRONT_OUTPUT_HORN,     // on UV cutoff, no current sense
  FRONT_OUTPUT_UV_VBAT,  // on UV cutoff, current sense ONLY
  FRONT_OUTPUT_5V_SPARE_1,
  FRONT_OUTPUT_5V_SPARE_2,
  FRONT_OUTPUT_SPARE_1,
  FRONT_OUTPUT_SPARE_2,
  FRONT_OUTPUT_SPARE_3,
  FRONT_OUTPUT_SPARE_4,  // on MCI's BTS7040
  FRONT_OUTPUT_SPARE_5,  // on rear fan 1's BTS7200 channel
  FRONT_OUTPUT_SPARE_6,  // on rear fan 2's BTS7200 channel

  // Outputs for rear power distribution
  REAR_OUTPUT_BMS,
  REAR_OUTPUT_MCI,
  REAR_OUTPUT_CHARGER,
  REAR_OUTPUT_SOLAR_SENSE,
  REAR_OUTPUT_REAR_CAMERA,
  REAR_OUTPUT_FAN_1,  // these are on the load switch, not UV cutoff
  REAR_OUTPUT_FAN_2,
  REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
  REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
  REAR_OUTPUT_BRAKE_LIGHT,
  REAR_OUTPUT_BPS_STROBE_LIGHT,
  REAR_OUTPUT_5V_SPARE_1,
  REAR_OUTPUT_5V_SPARE_2,
  REAR_OUTPUT_SPARE_1,
  REAR_OUTPUT_SPARE_2,
  REAR_OUTPUT_SPARE_3,
  REAR_OUTPUT_SPARE_4,   // on pedal's BTS7200 channel
  REAR_OUTPUT_SPARE_5,   // on steering's BTS7200 channel
  REAR_OUTPUT_SPARE_6,   // on right camera's BTS7200 channel
  REAR_OUTPUT_SPARE_7,   // on main pi's BTS7200 channel
  REAR_OUTPUT_SPARE_8,   // on driver display's BTS7200 channel
  REAR_OUTPUT_SPARE_9,   // on centre console's BTS7200 channel
  REAR_OUTPUT_SPARE_10,  // on rear display's BTS7200 channel

  NUM_OUTPUTS,
} Output;

typedef enum {
  OUTPUT_TYPE_IGNORE = 0,  // so that unspecified OutputSpecs default to ignore
  OUTPUT_TYPE_GPIO,
  OUTPUT_TYPE_BTS7200,
  OUTPUT_TYPE_BTS7040,
  NUM_OUTPUT_TYPES,
} OutputType;

typedef struct OutputGpioSpec {
  GpioAddress address;
} OutputGpioSpec;

// there is one OutputBts7200Info per BTS7200, they're pointed to by OutputBts7200Spec
typedef struct OutputBts7200Info {
  // We assume they're all on PCA9539R
  Pca9539rGpioAddress enable_0_pin;
  Pca9539rGpioAddress enable_1_pin;
  Pca9539rGpioAddress dsel_pin;
  uint8_t mux_selection;  // what should we select on the mux to read current from the BTS7200?
} OutputBts7200Info;

typedef struct OutputBts7200Spec {
  Bts7200Channel channel;
  OutputBts7200Info *bts7200_info;
} OutputBts7200Spec;

typedef struct OutputBts7040Spec {
  Pca9539rGpioAddress enable_pin;
  uint8_t mux_selection;  // what should we select on the mux to read current from the BTS7040?
  bool use_bts7004_scaling;
} OutputBts7040Spec;

typedef struct OutputSpec {
  OutputType type;
  bool on_front;
  union {
    OutputGpioSpec gpio_spec;
    OutputBts7200Spec bts7200_spec;
    OutputBts7040Spec bts7040_spec;
  };
} OutputSpec;

typedef struct OutputConfig {
  I2CPort i2c_port;

  I2CAddress *i2c_addresses;  // all i2c addresses on which there are PCA9539Rs
  uint8_t num_i2c_addresses;  // length of preceding array

  MuxAddress mux_address;
  GpioAddress mux_output_pin;
  GpioAddress mux_enable_pin;

  OutputSpec specs[NUM_OUTPUTS];
} OutputConfig;

typedef enum {
  OUTPUT_STATE_OFF = 0,
  OUTPUT_STATE_ON,
  NUM_OUTPUT_STATES,
} OutputState;

// Initialize the module.
StatusCode output_init(OutputConfig *config, bool is_front_power_distro);

// Set whether the output is on or off.
StatusCode output_set_state(Output output, OutputState state);

// Read the current that the output is drawing into |*current| in mA.
// STATUS_CODE_INVALID_ARGS is returned if current sense isn't supported by this output.
StatusCode output_read_current(Output output, uint16_t *current);

// Get the BTS7200 storage associated with an output, or NULL if it isn't OUTPUT_TYPE_BTS7200.
// For testing purposes, do not use outside tests.
Bts7200Storage *test_output_get_bts7200_storage(Output output);
