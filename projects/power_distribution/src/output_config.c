#include "output_config.h"

#include "pin_defs.h"

static const OutputBts7200Info s_centre_console_rear_display_front_bts7200 = {
  .enable_0_pin = FRONT_PIN_CENTRE_CONSOLE_EN,
  .enable_1_pin = FRONT_PIN_REAR_DISPLAY_EN,
  .dsel_pin = FRONT_PIN_CENTRE_CONSOLE_REAR_DISPLAY_DSEL,
  .mux_selection = FRONT_MUX_SEL_CENTRE_CONSOLE_REAR_DISPLAY,
};

static const OutputBts7200Info s_pedal_steering_front_bts7200 = {
  .enable_0_pin = FRONT_PIN_PEDAL_EN,
  .enable_1_pin = FRONT_PIN_STEERING_EN,
  .dsel_pin = FRONT_PIN_PEDAL_STEERING_DSEL,
  .mux_selection = FRONT_MUX_SEL_PEDAL_STEERING,
};

static const OutputBts7200Info s_left_right_camera_front_bts7200 = {
  .enable_0_pin = FRONT_PIN_LEFT_CAMERA_EN,
  .enable_1_pin = FRONT_PIN_RIGHT_CAMERA_EN,
  .dsel_pin = FRONT_PIN_LEFT_RIGHT_CAMERA_DSEL,
  .mux_selection = FRONT_MUX_SEL_LEFT_RIGHT_CAMERA,
};

static const OutputBts7200Info s_main_pi_driver_display_bts7200 = {
  .enable_0_pin = FRONT_PIN_MAIN_PI_EN,
  .enable_1_pin = FRONT_PIN_DRIVER_DISPLAY_EN,
  .dsel_pin = FRONT_PIN_MAIN_PI_DRIVER_DISPLAY_DSEL,
  .mux_selection = FRONT_MUX_SEL_MAIN_PI_DRIVER_DISPLAY,
};

// clang-format off
const OutputConfig COMBINED_OUTPUT_CONFIG = {
  .specs = {
    [FRONT_OUTPUT_CENTRE_CONSOLE] = {
      .type = OUTPUT_TYPE_BTS7200,
      .bts7200_spec = {
        .bts7200_info = &s_centre_console_rear_display_front_bts7200,
        .channel = 0,
      },
    },
    [FRONT_OUTPUT_PEDAL] = {
      .type = OUTPUT_TYPE_BTS7200,
      .bts7200_spec = {
        .bts7200_info = &s_pedal_steering_front_bts7200,
        .channel = 0,
      },
    },
    [FRONT_OUTPUT_STEERING] = {
      .type = OUTPUT_TYPE_BTS7200,
      .bts7200_spec = {
        .bts7200_info = &s_pedal_steering_front_bts7200,
        .channel = 1,
      },
    },
    [FRONT_OUTPUT_LEFT_CAMERA] = {
      .type = OUTPUT_TYPE_BTS7200,
      .bts7200_spec = {
        .bts7200_info = &s_left_right_camera_front_bts7200,
        .channel = 0,
      },
    },
    [FRONT_OUTPUT_RIGHT_CAMERA] = {
      .type = OUTPUT_TYPE_BTS7200,
      .bts7200_spec = {
        .bts7200_info = &s_left_right_camera_front_bts7200,
        .channel = 1,
      },
    },
    [FRONT_OUTPUT_DRIVER_DISPLAY] = {
      .type = OUTPUT_TYPE_BTS7200,
      .bts7200_spec = {
        .bts7200_info = &s_main_pi_driver_display_bts7200,
        .channel = 1,
      },
    },
    [FRONT_OUTPUT_SPEAKER] = {
      .type = OUTPUT_TYPE_BTS7040,
      .bts7040_spec = {
        .enable_pin = FRONT_PIN_SPEAKER_EN,
        .mux_selection = FRONT_MUX_SEL_SPEAKER,
      },
    },
  },
};
// clang-format on
