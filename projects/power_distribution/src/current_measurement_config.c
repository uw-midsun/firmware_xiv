#include "current_measurement_config.h"

#include "current_measurement.h"
#include "pin_defs.h"

// Definitions of the hardware configs declared in the header

// TODO(SOFT-396): maybe change this so it doesn't require counting

const PowerDistributionCurrentHardwareConfig FRONT_CURRENT_MEASUREMENT_HW_CONFIG = {
  .outputs_to_read = {
    FRONT_OUTPUT_CENTRE_CONSOLE,
    FRONT_OUTPUT_PEDAL,
    FRONT_OUTPUT_STEERING,
    FRONT_OUTPUT_LEFT_CAMERA,
    FRONT_OUTPUT_RIGHT_CAMERA,
    FRONT_OUTPUT_DRIVER_DISPLAY,
    FRONT_OUTPUT_INFOTAINMENT_DISPLAY,
    FRONT_OUTPUT_REAR_DISPLAY,
    FRONT_OUTPUT_LEFT_DISPLAY,
    FRONT_OUTPUT_RIGHT_DISPLAY,
    FRONT_OUTPUT_MAIN_PI,
    FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT,
    FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT,
    FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS,
    FRONT_OUTPUT_SPEAKER,
    FRONT_OUTPUT_UV_VBAT,
  },
  .num_outputs_to_read = 16,
};

const PowerDistributionCurrentHardwareConfig REAR_CURRENT_MEASUREMENT_HW_CONFIG = {
  .outputs_to_read = {
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
  },
  .num_outputs_to_read = 11,
};
