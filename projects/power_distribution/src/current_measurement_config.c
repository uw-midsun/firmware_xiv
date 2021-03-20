#include "current_measurement_config.h"

#include "current_measurement.h"
#include "pin_defs.h"

// Definitions of the hardware configs declared in the header

// TODO(SOFT-396): UV_VBAT_IS

const PowerDistributionCurrentHardwareConfig FRONT_CURRENT_MEASUREMENT_HW_CONFIG = {
  .i2c_port = PD_I2C_PORT,
  .dsel_i2c_addresses =
      (I2CAddress[]){
          PD_PCA9539R_I2C_ADDRESS_0,  //
          PD_PCA9539R_I2C_ADDRESS_1,  //
      },
  .num_dsel_i2c_addresses = 2,
  .bts7200s =
      (PowerDistributionBts7200Data[]){
          {
              .dsel_pin = FRONT_PIN_LEFT_RIGHT_CAMERA_DSEL,
              .en0_pin = FRONT_PIN_LEFT_CAMERA_EN,
              .en1_pin = FRONT_PIN_RIGHT_CAMERA_EN,
              .current_0 = FRONT_CURRENT_LEFT_CAMERA,
              .current_1 = FRONT_CURRENT_RIGHT_CAMERA,
              .mux_selection = FRONT_MUX_SEL_LEFT_RIGHT_CAMERA,
          },
          {
              .dsel_pin = FRONT_PIN_CENTRE_CONSOLE_REAR_DISPLAY_DSEL,
              .en0_pin = FRONT_PIN_CENTRE_CONSOLE_EN,
              .en1_pin = FRONT_PIN_REAR_DISPLAY_EN,
              .current_0 = FRONT_CURRENT_CENTRE_CONSOLE,
              .current_1 = FRONT_CURRENT_REAR_DISPLAY,
              .mux_selection = FRONT_MUX_SEL_CENTRE_CONSOLE_REAR_DISPLAY,
          },
          {
              .dsel_pin = FRONT_PIN_LEFT_RIGHT_DISPLAY_DSEL,
              .en0_pin = FRONT_PIN_LEFT_DISPLAY_EN,
              .en1_pin = FRONT_PIN_RIGHT_DISPLAY_EN,
              .current_0 = FRONT_CURRENT_LEFT_DISPLAY,
              .current_1 = FRONT_CURRENT_RIGHT_DISPLAY,
              .mux_selection = FRONT_MUX_SEL_LEFT_RIGHT_DISPLAY,
          },
          {
              .dsel_pin = FRONT_PIN_FRONT_LEFT_RIGHT_TURN_LIGHT_DSEL,
              .en0_pin = FRONT_PIN_FRONT_LEFT_TURN_LIGHT_EN,
              .en1_pin = FRONT_PIN_FRONT_RIGHT_TURN_LIGHT_EN,
              .current_0 = FRONT_CURRENT_LEFT_FRONT_TURN_LIGHT,
              .current_1 = FRONT_CURRENT_RIGHT_FRONT_TURN_LIGHT,
              .mux_selection = FRONT_MUX_SEL_FRONT_LEFT_RIGHT_TURN_LIGHT,
          },
          {
              .dsel_pin = FRONT_PIN_MAIN_PI_DRIVER_DISPLAY_DSEL,
              .en0_pin = FRONT_PIN_MAIN_PI_EN,
              .en1_pin = FRONT_PIN_DRIVER_DISPLAY_EN,
              .current_0 = FRONT_CURRENT_MAIN_PI,
              .current_1 = FRONT_CURRENT_DRIVER_DISPLAY,
              .mux_selection = FRONT_MUX_SEL_MAIN_PI_DRIVER_DISPLAY,
          },
          {
              .dsel_pin = FRONT_PIN_PEDAL_STEERING_DSEL,
              .en0_pin = FRONT_PIN_PEDAL_EN,
              .en1_pin = FRONT_PIN_STEERING_EN,
              .current_0 = FRONT_CURRENT_PEDAL,
              .current_1 = FRONT_CURRENT_STEERING,
              .mux_selection = FRONT_MUX_SEL_PEDAL_STEERING,
          },
          //   {
          //       .dsel_pin = FRONT_PIN_FAN_1_2_DSEL,
          //       .en0_pin = FRONT_PIN_FAN_1_EN,
          //       .en1_pin = FRONT_PIN_FAN_2_EN,
          //       .current_0 = FRONT_CURRENT_FAN_1,
          //       .current_1 = FRONT_CURRENT_FAN_2,
          //       .mux_selection = FRONT_MUX_SEL_FAN_1_2,
          //   },
      },
  .num_bts7200_channels = 7,
  .bts7040s =
      (PowerDistributionBts7040Data[]){
          {
              .en_pin = FRONT_PIN_INFOTAINMENT_DISPLAY_EN,
              .current = FRONT_CURRENT_INFOTAINMENT_DISPLAY,
              .mux_selection = FRONT_MUX_SEL_INFOTAINMENT_DISPLAY,
          },
          {
              .en_pin = FRONT_PIN_DAYTIME_RUNNING_LIGHTS_EN,
              .current = FRONT_CURRENT_DAYTIME_RUNNING_LIGHTS,
              .mux_selection = FRONT_MUX_SEL_DAYTIME_RUNNING_LIGHTS,
          },
          {
              .en_pin = FRONT_PIN_SPEAKER_EN,
              .current = FRONT_CURRENT_SPEAKER,
              .mux_selection = FRONT_MUX_SEL_SPEAKER,
          },
      },
  .num_bts7040_channels = 3,
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  PD_MUX_SEL1_PIN,  //
                  PD_MUX_SEL2_PIN,  //
                  PD_MUX_SEL3_PIN,  //
                  PD_MUX_SEL4_PIN,  //
              },
      },
  .mux_output_pin = PD_MUX_OUTPUT_PIN,  //
  .mux_enable_pin = PD_MUX_ENABLE_PIN,  //
};

const PowerDistributionCurrentHardwareConfig REAR_CURRENT_MEASUREMENT_HW_CONFIG = {
  .i2c_port = PD_I2C_PORT,
  .dsel_i2c_addresses =
      (I2CAddress[]){
          PD_PCA9539R_I2C_ADDRESS_0,  //
          PD_PCA9539R_I2C_ADDRESS_1,  //
      },
  .num_dsel_i2c_addresses = 2,
  .bts7200s =
      (PowerDistributionBts7200Data[]){
          {
              .dsel_pin = REAR_PIN_CHARGER_STROBE_LIGHT_DSEL,
              .en0_pin = REAR_PIN_CHARGER_EN,
              .en1_pin = REAR_PIN_STROBE_LIGHT_EN,
              .current_0 = REAR_CURRENT_CHARGER,
              .current_1 = REAR_CURRENT_STROBE,
              .mux_selection = REAR_MUX_SEL_CHARGER_STROBE_LIGHT,
          },
          {
              .dsel_pin = REAR_PIN_REAR_LEFT_RIGHT_TURN_LIGHT_DSEL,
              .en0_pin = REAR_PIN_REAR_LEFT_TURN_LIGHT_EN,
              .en1_pin = REAR_PIN_REAR_RIGHT_TURN_LIGHT_EN,
              .current_0 = REAR_CURRENT_LEFT_REAR_TURN_LIGHT,
              .current_1 = REAR_CURRENT_RIGHT_REAR_TURN_LIGHT,
              .mux_selection = REAR_MUX_SEL_REAR_LEFT_RIGHT_TURN_LIGHT,
          },
          {
              .dsel_pin = REAR_PIN_REAR_CAMERA_SPARE_6_DSEL,
              .en0_pin = REAR_PIN_REAR_CAMERA_EN,
              .en1_pin = REAR_PIN_SPARE_6_EN,
              .current_0 = REAR_CURRENT_REAR_CAMERA,
              .current_1 = REAR_CURRENT_SPARE_6,
              .mux_selection = REAR_MUX_SEL_REAR_CAMERA_SPARE_6,
          },
          {
              .dsel_pin = REAR_PIN_FAN_1_2_DSEL,
              .en0_pin = REAR_PIN_FAN_1_EN,
              .en1_pin = REAR_PIN_FAN_2_EN,
              .current_0 = REAR_CURRENT_FAN_1,
              .current_1 = REAR_CURRENT_FAN_2,
              .mux_selection = REAR_MUX_SEL_FAN_1_2,
          },
      },
  .num_bts7200_channels = 4,
  .bts7040s =
      (PowerDistributionBts7040Data[]){
          {
              .en_pin = REAR_PIN_BMS_EN,
              .current = REAR_CURRENT_BMS,
              .mux_selection = REAR_MUX_SEL_BMS,
          },
          {
              .en_pin = REAR_PIN_MCI_EN,
              .current = REAR_CURRENT_MCI,
              .mux_selection = REAR_MUX_SEL_MCI,
          },
          {
              .en_pin = REAR_PIN_SOLAR_SENSE_EN,
              .current = REAR_CURRENT_SOLAR_SENSE,
              .mux_selection = REAR_MUX_SEL_SOLAR_SENSE,
          },
          {
              .en_pin = REAR_PIN_BRAKE_LIGHT_EN,
              .current = REAR_CURRENT_BRAKE_LIGHT,
              .mux_selection = REAR_MUX_SEL_BRAKE_LIGHT,
          },
      },
  .num_bts7040_channels = 4,
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  PD_MUX_SEL1_PIN,  //
                  PD_MUX_SEL2_PIN,  //
                  PD_MUX_SEL3_PIN,  //
                  PD_MUX_SEL4_PIN,  //
              },
      },
  .mux_output_pin = PD_MUX_OUTPUT_PIN,  //
  .mux_enable_pin = PD_MUX_ENABLE_PIN,  //
};
