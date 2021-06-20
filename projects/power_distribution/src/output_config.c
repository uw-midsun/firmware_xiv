#include "output_config.h"

#include <stdbool.h>

#include "i2c.h"
#include "pin_defs.h"

// used for a hacky workaround to specify UV_VBAT as BTS7040 with only current sense enabled
#define NONEXISTENT_PCA9539R_PIN \
  { 0xff, PCA9539R_PIN_IO0_0 }

// Front power distribution BTS7200s

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

static const OutputBts7200Info s_main_pi_driver_display_front_bts7200 = {
  .enable_0_pin = FRONT_PIN_MAIN_PI_EN,
  .enable_1_pin = FRONT_PIN_DRIVER_DISPLAY_EN,
  .dsel_pin = FRONT_PIN_MAIN_PI_DRIVER_DISPLAY_DSEL,
  .mux_selection = FRONT_MUX_SEL_MAIN_PI_DRIVER_DISPLAY,
};

static const OutputBts7200Info s_left_right_display_front_bts7200 = {
  .enable_0_pin = FRONT_PIN_LEFT_DISPLAY_EN,
  .enable_1_pin = FRONT_PIN_RIGHT_DISPLAY_EN,
  .dsel_pin = FRONT_PIN_LEFT_RIGHT_DISPLAY_DSEL,
  .mux_selection = FRONT_MUX_SEL_LEFT_RIGHT_DISPLAY,
};

static const OutputBts7200Info s_front_left_right_turn_light_front_bts7200 = {
  .enable_0_pin = FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN,
  .enable_1_pin = FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN,
  .dsel_pin = FRONT_PIN_FRONT_LEFT_RIGHT_TURN_LIGHT_DSEL,
  .mux_selection = FRONT_MUX_SEL_FRONT_LEFT_RIGHT_TURN_LIGHT,
};

static const OutputBts7200Info s_5v_spare_front_bts7200_info = {
  .enable_0_pin = FRONT_PIN_5V_SPARE_1_EN,
  .enable_1_pin = FRONT_PIN_5V_SPARE_2_EN,
  .dsel_pin = FRONT_PIN_5V_SPARE_1_2_DSEL,
  .mux_selection = FRONT_MUX_SEL_5V_SPARES,
};

static const OutputBts7200Info s_spare_2_3_front_bts7200_info = {
  .enable_0_pin = FRONT_PIN_SPARE_2_EN,
  .enable_1_pin = FRONT_PIN_SPARE_3_EN,
  .dsel_pin = FRONT_PIN_SPARE_2_3_DSEL,
  .mux_selection = FRONT_MUX_SEL_SPARE_2_3,
};

static const OutputBts7200Info s_spare_5_6_front_bts7200_info = {
  .enable_0_pin = FRONT_PIN_SPARE_5_EN,
  .enable_1_pin = FRONT_PIN_SPARE_6_EN,
  .dsel_pin = FRONT_PIN_SPARE_5_6_DSEL,
  .mux_selection = FRONT_MUX_SEL_SPARE_5_6,
};

// Rear power distribution BTS7200s

static const OutputBts7200Info s_charger_strobe_rear_bts7200 = {
  .enable_0_pin = REAR_PIN_CHARGER_EN,
  .enable_1_pin = REAR_PIN_STROBE_LIGHT_EN,
  .dsel_pin = REAR_PIN_CHARGER_STROBE_LIGHT_DSEL,
  .mux_selection = REAR_MUX_SEL_CHARGER_STROBE_LIGHT,
};

static const OutputBts7200Info s_rear_camera_spare_6_rear_bts7200 = {
  .enable_0_pin = REAR_PIN_REAR_CAMERA_EN,
  .enable_1_pin = REAR_PIN_SPARE_6_EN,
  .dsel_pin = REAR_PIN_REAR_CAMERA_SPARE_6_DSEL,
  .mux_selection = REAR_MUX_SEL_REAR_CAMERA_SPARE_6,
};

static const OutputBts7200Info s_rear_left_right_turn_light_rear_bts7200 = {
  .enable_0_pin = REAR_PIN_REAR_LEFT_TURN_LIGHT_EN,
  .enable_1_pin = REAR_PIN_REAR_RIGHT_TURN_LIGHT_EN,
  .dsel_pin = REAR_PIN_REAR_LEFT_RIGHT_TURN_LIGHT_DSEL,
  .mux_selection = REAR_MUX_SEL_REAR_LEFT_RIGHT_TURN_LIGHT,
};

static const OutputBts7200Info s_fan_1_2_rear_bts7200 = {
  .enable_0_pin = REAR_PIN_FAN_1_EN,
  .enable_1_pin = REAR_PIN_FAN_2_EN,
  .dsel_pin = REAR_PIN_FAN_1_2_DSEL,
  .mux_selection = REAR_MUX_SEL_FAN_1_2,
};

static const OutputBts7200Info s_5v_spare_rear_bts7200_info = {
  .enable_0_pin = REAR_PIN_5V_SPARE_1_EN,
  .enable_1_pin = REAR_PIN_5V_SPARE_2_EN,
  .dsel_pin = REAR_PIN_5V_SPARE_1_2_DSEL,
  .mux_selection = REAR_MUX_SEL_5V_SPARES,
};

static const OutputBts7200Info s_spare_2_3_rear_bts7200_info = {
  .enable_0_pin = REAR_PIN_SPARE_2_EN,
  .enable_1_pin = REAR_PIN_SPARE_3_EN,
  .dsel_pin = REAR_PIN_SPARE_2_3_DSEL,
  .mux_selection = REAR_MUX_SEL_SPARE_2_3,
};

static const OutputBts7200Info s_spare_4_5_rear_bts7200_info = {
  .enable_0_pin = REAR_PIN_SPARE_4_EN,
  .enable_1_pin = REAR_PIN_SPARE_5_EN,
  .dsel_pin = REAR_PIN_SPARE_4_5_DSEL,
  .mux_selection = REAR_MUX_SEL_SPARE_4_5,
};

static const OutputBts7200Info s_spare_7_8_rear_bts7200_info = {
  .enable_0_pin = REAR_PIN_SPARE_7_EN,
  .enable_1_pin = REAR_PIN_SPARE_8_EN,
  .dsel_pin = REAR_PIN_SPARE_7_8_DSEL,
  .mux_selection = REAR_MUX_SEL_SPARE_7_8,
};

static const OutputBts7200Info s_spare_9_10_rear_bts7200_info = {
  .enable_0_pin = REAR_PIN_SPARE_9_EN,
  .enable_1_pin = REAR_PIN_SPARE_10_EN,
  .dsel_pin = REAR_PIN_SPARE_9_10_DSEL,
  .mux_selection = REAR_MUX_SEL_SPARE_9_10,
};

// clang-format off
const OutputConfig g_combined_output_config = {
  .i2c_port = PD_I2C_PORT,
  .i2c_addresses = (I2CAddress[]){
    PD_PCA9539R_I2C_ADDRESS_0,
    PD_PCA9539R_I2C_ADDRESS_1,
  },
  .num_i2c_addresses = 2,
  .mux_address = {
    .bit_width = 4,
    .sel_pins = {
      PD_MUX_SEL1_PIN,
      PD_MUX_SEL2_PIN,
      PD_MUX_SEL3_PIN,
      PD_MUX_SEL4_PIN,
    },
  },
  .mux_output_pin = PD_MUX_OUTPUT_PIN,
  .mux_enable_pin = PD_MUX_ENABLE_PIN,
  .specs = {
    // Front power distribution outputs
    [FRONT_OUTPUT_CENTRE_CONSOLE] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_centre_console_rear_display_front_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_PEDAL] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_pedal_steering_front_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_STEERING] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_pedal_steering_front_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_LEFT_CAMERA] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_left_right_camera_front_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_RIGHT_CAMERA] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_left_right_camera_front_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_DRIVER_DISPLAY] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_main_pi_driver_display_front_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_INFOTAINMENT_DISPLAY] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = true,
      .bts7040_spec = {
        .enable_pin = FRONT_PIN_INFOTAINMENT_DISPLAY_EN,
        .mux_selection = FRONT_MUX_SEL_INFOTAINMENT_DISPLAY,
      },
    },
    [FRONT_OUTPUT_REAR_DISPLAY] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_centre_console_rear_display_front_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_LEFT_DISPLAY] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_left_right_display_front_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_RIGHT_DISPLAY] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_left_right_display_front_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_MAIN_PI] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_main_pi_driver_display_front_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_LEFT_FRONT_TURN_LIGHT] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_front_left_right_turn_light_front_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_RIGHT_FRONT_TURN_LIGHT] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_front_left_right_turn_light_front_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_DAYTIME_RUNNING_LIGHTS] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = true,
      .bts7040_spec = {
        .enable_pin = FRONT_PIN_DAYTIME_RUNNING_LIGHTS_EN,
        .mux_selection = FRONT_MUX_SEL_DAYTIME_RUNNING_LIGHTS,
      },
    },
    [FRONT_OUTPUT_SPEAKER] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = true,
      .bts7040_spec = {
        .enable_pin = FRONT_PIN_SPEAKER_EN,
        .mux_selection = FRONT_MUX_SEL_SPEAKER,
      },
    },
    [FRONT_OUTPUT_HORN] = {
      .type = OUTPUT_TYPE_GPIO,
      .on_front = true,
      .gpio_spec = {
        .address = FRONT_PIN_HORN_EN,
      },
    },
    [FRONT_OUTPUT_FAN] = {
      .type = OUTPUT_TYPE_GPIO,
      .on_front = true,
      .gpio_spec = {
        .address = FRONT_PIN_FAN_EN,
      },
    },
    [FRONT_OUTPUT_UV_VBAT] = {
      // We don't control UV VBAT, but we read its current through a BTS7040 (actually a BTS7004)
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = true,
      .bts7040_spec = {
        // hacky workaround to use the BTS7040 driver for current reading without a valid en pin
        .enable_pin = NONEXISTENT_PCA9539R_PIN,
        .mux_selection = FRONT_MUX_SEL_UV_VBAT,
      },
    },
    [FRONT_OUTPUT_5V_SPARE_1] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_5v_spare_front_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_5V_SPARE_2] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_5v_spare_front_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_SPARE_1] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = true,
      .bts7040_spec = {
        .enable_pin = FRONT_PIN_SPARE_1_EN,
        .mux_selection = FRONT_MUX_SEL_SPARE_1,
      },
    },
    [FRONT_OUTPUT_SPARE_2] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_spare_2_3_front_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_SPARE_3] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_spare_2_3_front_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [FRONT_OUTPUT_SPARE_4] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = true,
      .bts7040_spec = {
        .enable_pin = FRONT_PIN_SPARE_4_EN,
        .mux_selection = FRONT_MUX_SEL_SPARE_4,
      },
    },
    [FRONT_OUTPUT_SPARE_5] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_spare_5_6_front_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [FRONT_OUTPUT_SPARE_6] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = true,
      .bts7200_spec = {
        .bts7200_info = &s_spare_5_6_front_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },

    // Rear power distribution outputs
    [REAR_OUTPUT_BMS] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = false,
      .bts7040_spec = {
        .enable_pin = REAR_PIN_BMS_EN,
        .mux_selection = REAR_MUX_SEL_BMS,
      },
    },
    [REAR_OUTPUT_MCI] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = false,
      .bts7040_spec = {
        .enable_pin = REAR_PIN_MCI_EN,
        .mux_selection = REAR_MUX_SEL_MCI,
      },
    },
    [REAR_OUTPUT_CHARGER] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_charger_strobe_rear_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_SOLAR_SENSE] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = false,
      .bts7040_spec = {
        .enable_pin = REAR_PIN_SOLAR_SENSE_EN,
        .mux_selection = REAR_MUX_SEL_SOLAR_SENSE,
      },
    },
    [REAR_OUTPUT_REAR_CAMERA] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_rear_camera_spare_6_rear_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_LEFT_REAR_TURN_LIGHT] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_rear_left_right_turn_light_rear_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_RIGHT_REAR_TURN_LIGHT] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_rear_left_right_turn_light_rear_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_BRAKE_LIGHT] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = false,
      .bts7040_spec = {
        .enable_pin = REAR_PIN_BRAKE_LIGHT_EN,
        .mux_selection = REAR_MUX_SEL_BRAKE_LIGHT,
      },
    },
    [REAR_OUTPUT_BPS_STROBE_LIGHT] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_charger_strobe_rear_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_FAN_1] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_fan_1_2_rear_bts7200,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_FAN_2] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_fan_1_2_rear_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_5V_SPARE_1] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_5v_spare_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_5V_SPARE_2] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_5v_spare_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_SPARE_1] = {
      .type = OUTPUT_TYPE_BTS7040,
      .on_front = false,
      .bts7040_spec = {
        .enable_pin = REAR_PIN_SPARE_1_EN,
        .mux_selection = REAR_MUX_SEL_SPARE_1,
      },
    },
    [REAR_OUTPUT_SPARE_2] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_2_3_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_SPARE_3] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_2_3_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_SPARE_4] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_4_5_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_SPARE_5] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_4_5_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_SPARE_6] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_rear_camera_spare_6_rear_bts7200,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_SPARE_7] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_7_8_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_SPARE_8] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_7_8_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },
    [REAR_OUTPUT_SPARE_9] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_9_10_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_0,
      },
    },
    [REAR_OUTPUT_SPARE_10] = {
      .type = OUTPUT_TYPE_BTS7200,
      .on_front = false,
      .bts7200_spec = {
        .bts7200_info = &s_spare_9_10_rear_bts7200_info,
        .channel = BTS7200_CHANNEL_1,
      },
    },
  },
};
// clang-format on
