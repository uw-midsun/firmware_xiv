#pragma once

#include <stdint.h>

#include "status.h"

// General-use module for manipulating the outputs that power distribution controls.
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
  FRONT_OUTPUT_INFOTAINMENT_DISPLAY, // aka main display
  FRONT_OUTPUT_REAR_DISPLAY,
  FRONT_OUTPUT_LEFT_DISPLAY,
  FRONT_OUTPUT_RIGHT_DISPLAY,
  FRONT_OUTPUT_MAIN_PI, // driver display + telemetry pi
  FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
  FRONT_OUTPUT_SPEAKER,
  FRONT_OUTPUT_HORN,
  FRONT_OUTPUT_FAN_1,
  FRONT_OUTPUT_FAN_2,
  FRONT_OUTPUT_5V_SPARE_1,
  FRONT_OUTPUT_5V_SPARE_2,
  FRONT_OUTPUT_SPARE_1,
  FRONT_OUTPUT_SPARE_2,
  FRONT_OUTPUT_SPARE_3,
  FRONT_OUTPUT_SPARE_4, // MCI load switch port

  // Outputs for rear power distribution
  REAR_OUTPUT_BMS,
  REAR_OUTPUT_MCI,
  REAR_OUTPUT_CHARGER,
  REAR_OUTPUT_SOLAR_SENSE,
  REAR_OUTPUT_TELEMETRY,
  REAR_OUTPUT_REAR_CAMERA,
  REAR_OUTPUT_LEFT_REAR_TURN_LIGHT,
  REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT,
  REAR_OUTPUT_BRAKE_LIGHT,
  REAR_OUTPUT_STROBE,
  REAR_OUTPUT_FAN_1,
  REAR_OUTPUT_FAN_2,
  REAR_OUTPUT_5V_SPARE_1,
  REAR_OUTPUT_5V_SPARE_2,
  REAR_OUTPUT_SPARE_1,
  REAR_OUTPUT_SPARE_2,
  REAR_OUTPUT_SPARE_3,
  REAR_OUTPUT_SPARE_4, // steering load switch port
  REAR_OUTPUT_SPARE_5, // pedal load switch port
  REAR_OUTPUT_SPARE_6, // right camera load switch port
  REAR_OUTPUT_SPARE_7, // main pi (driver display/telemetry pi) load switch port
  REAR_OUTPUT_SPARE_8, // driver display load switch port
  REAR_OUTPUT_SPARE_9, // centre console load switch port
  REAR_OUTPUT_SPARE_10, // rear display load switch port
} Output;

typedef enum {
  OUTPUT_STATE_OFF = 0,
  OUTPUT_STATE_ON,
  NUM_OUTPUT_STATES,
} OutputState;

// Set whether the output is on or off.
StatusCode output_set_state(Output output, OutputState state);

// Read the current that the output is drawing into |*current| in mA.
// STATUS_CODE_INVALID_ARGS is returned if current sense isn't supported by this output.
StatusCode output_read_current(Output output, uint16_t *current);
