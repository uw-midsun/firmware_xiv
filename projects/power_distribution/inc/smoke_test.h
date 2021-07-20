#pragma once

#include "output.h"

// Anything uncommented will be turned on PD starts up.
// Outputs are named semantically rather than by port/pin - it's probably easiest to look up the
// pins by matching the names to the schematics, but the output -> load switch mapping is in
// output_config.c if you need to debug.

// These will be turned on when front PD is detected.
const Output g_turn_on_front[] = {
  FRONT_OUTPUT_CENTRE_CONSOLE,
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
  FRONT_OUTPUT_FAN,  // on UV cutoff via gpio pin (not on load switch)
  FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
  FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
  FRONT_OUTPUT_HORN,  // on UV cutoff via gpio pin (not on load switch)
  FRONT_OUTPUT_5V_SPARE_1,
  FRONT_OUTPUT_5V_SPARE_2,
  FRONT_OUTPUT_SPARE_1,
  FRONT_OUTPUT_SPARE_2,
  FRONT_OUTPUT_SPARE_3,
  FRONT_OUTPUT_SPARE_4,  // on MCI's BTS7040
  FRONT_OUTPUT_SPARE_5,  // on rear fan 1's BTS7200 channel
  FRONT_OUTPUT_SPARE_6,  // on rear fan 2's BTS7200 channel
};

// These will be turned on when rear PD is detected.
const Output g_turn_on_rear[] = {
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
};
