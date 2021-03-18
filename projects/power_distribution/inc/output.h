#pragma once

#include <stdint.h>

#include "gpio.h"
#include "pca9539r_gpio_expander.h"
#include "status.h"

// General-purpose module for manipulating the outputs that power distribution controls.
// An output is an abstraction of "something that PD can turn on and off".
// This module provides a uniform interface for manipulating outputs implemented through GPIO or
// an IO expander, and through a BTS7200 or BTS7040 load switch or not.

// TODO(SOFT-396): update the currents confluence page
typedef enum {
  // Outputs for front power distribution
  FRONT_OUTPUT_CENTRE_CONSOLE = 0,
  FRONT_OUTPUT_PEDAL,
  FRONT_OUTPUT_STEERING,
  FRONT_OUTPUT_LEFT_CAMERA,
  FRONT_OUTPUT_RIGHT_CAMERA,
  FRONT_OUTPUT_DRIVER_DISPLAY,
  FRONT_OUTPUT_INFOTAINMENT_DISPLAY,  // aka main display
  FRONT_OUTPUT_REAR_DISPLAY,
  FRONT_OUTPUT_LEFT_DISPLAY,
  FRONT_OUTPUT_RIGHT_DISPLAY,
  FRONT_OUTPUT_MAIN_PI,  // driver display + telemetry pi
  FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
  FRONT_OUTPUT_SPEAKER,
  FRONT_OUTPUT_HORN,
  // FRONT_OUTPUT_FRONT_FAN? TODO(SOFT-396)
  FRONT_OUTPUT_FAN_1,
  FRONT_OUTPUT_FAN_2,
  // spares omitted

  // Outputs for rear power distribution
  REAR_OUTPUT_BMS,
  REAR_OUTPUT_MCI,
  REAR_OUTPUT_CHARGER,
  REAR_OUTPUT_SOLAR_SENSE,
  REAR_OUTPUT_REAR_CAMERA,
  REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
  REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
  REAR_OUTPUT_BRAKE_LIGHT,
  REAR_OUTPUT_STROBE_LIGHT,
  REAR_OUTPUT_FAN_1,
  REAR_OUTPUT_FAN_2,
  // spares omitted

  NUM_OUTPUTS,
} Output;

// these structs should really go in a separate file, like output_impl.h or something
typedef enum {
  OUTPUT_TYPE_IGNORE = 0, // so that unspecified OutputSpecs default to ignore
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
  uint8_t channel;  // 0 or 1, which enable pin/output channel is it? TODO - separate type?
  OutputBts7200Info *bts7200_info;
} OutputBts7200Spec;

typedef struct OutputBts7040Spec {
  Pca9539rGpioAddress enable_pin;
  uint8_t mux_selection;  // what should we select on the mux to read current from the BTS7040?
} OutputBts7040Spec;

typedef struct OutputSpec {
  OutputType type;
  union {
    OutputGpioSpec gpio_spec;
    OutputBts7200Spec bts7200_spec;
    OutputBts7040Spec bts7040_spec;
  };
} OutputSpec;

typedef struct OutputConfig {
  OutputSpec specs[NUM_OUTPUTS];
} OutputConfig;

typedef enum {
  OUTPUT_STATE_OFF = 0,
  OUTPUT_STATE_ON,
  NUM_OUTPUT_STATES,
} OutputState;

// Initialize the module.
StatusCode output_init(OutputConfig *config);

// Set whether the output is on or off.
StatusCode output_set_state(Output output, OutputState state);

// Read the current that the output is drawing into |*current| in mA.
// STATUS_CODE_INVALID_ARGS is returned if current sense isn't supported by this output.
StatusCode output_read_current(Output output, uint16_t *current);
