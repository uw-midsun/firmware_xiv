#include "current_measurement_config.h"
#include "pin_defs.h"

// Definitions of the hardware configs declared in the header

#define POWER_DISTRIBUTION_I2C_PORT I2C_PORT_2

const PowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG = {
  .i2c_port = POWER_DISTRIBUTION_I2C_PORT,
  .dsel_i2c_addresses =
      (I2CAddress[]){
          POWER_DISTRIBUTION_I2C_ADDRESS_0,  //
          POWER_DISTRIBUTION_I2C_ADDRESS_1,  //
      },
  .num_dsel_i2c_addresses = 2,
  .bts7200s = (PowerDistributionBts7200Data[]){ {
                                                    .dsel_pin = FRONT_PIN_LEFT_RIGHT_CAM_DSEL,
                                                    .en0_pin = FRONT_PIN_LEFT_CAMERA_EN,
                                                    .en1_pin = FRONT_PIN_RIGHT_CAMERA_EN,
                                                    .mux_selection = 12,
                                                },
                                                {
                                                    .dsel_pin = FRONT_PIN_DVR_REAR_DISP_DSEL,
                                                    .en0_pin = FRONT_PIN_DVR_DISP_EN,
                                                    .en1_pin = FRONT_PIN_REAR_DISP_EN,
                                                    .mux_selection = 0,
                                                },
                                                {
                                                    .dsel_pin = FRONT_PIN_L_R_DVR_DISP_DSEL,
                                                    .en0_pin = FRONT_PIN_LEFT_DISPLAY_EN,
                                                    .en1_pin = FRONT_PIN_RIGHT_DISPLAY_EN,
                                                    .mux_selection = 4,
                                                },
                                                {
                                                    .dsel_pin = FRONT_PIN_FRONT_TURN_LIGHT_DSEL,
                                                    .en0_pin = FRONT_PIN_FRONT_LEFT_TURN_EN,
                                                    .en1_pin = FRONT_PIN_FRONT_RIGHT_TURN_EN,
                                                    .mux_selection = 9,
                                                },
                                                {
                                                    .dsel_pin = FRONT_PIN_MAIN_REAR_PI_DSEL,
                                                    .en0_pin = FRONT_PIN_MAIN_PI_B_EN,
                                                    .en1_pin = FRONT_PIN_REAR_PI_EN,
                                                    .mux_selection = 1,
                                                },
                                                {
                                                    .dsel_pin = FRONT_PIN_5V_SPARE_DSEL,
                                                    .en0_pin = FRONT_PIN_5V_SPARE_1_EN,
                                                    .en1_pin = FRONT_PIN_5V_SPARE_2_EN,
                                                    .mux_selection = 2,
                                                },
                                                {
                                                    .dsel_pin = FRONT_PIN_SPARE_2_CTR_CONSL_DSEL,
                                                    .en0_pin = FRONT_PIN_SPARE_2_EN,
                                                    .en1_pin = FRONT_PIN_CTR_CONSL_EN,
                                                    .mux_selection = 7,
                                                },
                                                {
                                                    .dsel_pin = FRONT_PIN_STR_PDL_DSEL,
                                                    .en0_pin = FRONT_PIN_STEERING_EN,
                                                    .en1_pin = FRONT_PIN_CTR_CONSL_EN,
                                                    .mux_selection = 6,
                                                } },
  .num_bts7200_channels = 8,
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  { .port = GPIO_PORT_A, .pin = 6 },  //
                  { .port = GPIO_PORT_A, .pin = 5 },  //
                  { .port = GPIO_PORT_A, .pin = 4 },  //
                  { .port = GPIO_PORT_A, .pin = 3 },  //
              },
      },
  .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },  //
  .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },  //
};

// This is based on https://uwmidsun.atlassian.net/wiki/x/GgODP, assuming that the currents in
// each row are connected to the same BTS7200s/BTS7040s (on the same pins with same mux selections).
const PowerDistributionCurrentHardwareConfig REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG = {
  .i2c_port = POWER_DISTRIBUTION_I2C_PORT,
  .dsel_i2c_addresses =
      (I2CAddress[]){
          POWER_DISTRIBUTION_I2C_ADDRESS_0,  //
          POWER_DISTRIBUTION_I2C_ADDRESS_1,  //
      },
  .num_dsel_i2c_addresses = 2,
  .bts7200s =
      (PowerDistributionBts7200Data[]){
          {
              .dsel_pin = REAR_PIN_SOLAR_TELEMETRY_DSEL,
              .en0_pin = REAR_PIN_SOLAR_SENSE_EN,
              .en1_pin = REAR_PIN_TELEMETRY_EN,
              .mux_selection = 7,
          },
          {
              .dsel_pin = REAR_PIN_STROBE_CTR_BRK_DSEL,
              .en0_pin = REAR_PIN_STROBE_LIGHT_EN,
              .en1_pin = REAR_PIN_CENTER_BRAKE_LIGHT_EN,
              .mux_selection = 9,
          },
          {
              .dsel_pin = REAR_PIN_REAR_TURN_LIGHT_DSEL,
              .en0_pin = REAR_PIN_REAR_LEFT_TURN_EN,
              .en1_pin = REAR_PIN_REAR_RIGHT_TURN_EN,
              .mux_selection = 0,
          },
          {
              .dsel_pin = REAR_PIN_REAR_BRAKE_LIGHT_DSEL,
              .en0_pin = REAR_PIN_REAR_LEFT_BRAKEL_EN,
              .en1_pin = REAR_PIN_REAR_RIGHT_BRAKEL_EN,
              .mux_selection = 4,
          },
          {
              .dsel_pin = REAR_PIN_CAM_SPARE_10_DSEL,
              .en0_pin = REAR_PIN_REAR_CAMERA_EN,
              .en1_pin = REAR_PIN_SPARE_10_EN,
              .mux_selection = 12,
          },
          {
              .dsel_pin = REAR_PIN_SPARE_1_2_DSEL,
              .en0_pin = REAR_PIN_SPARE_1_EN,
              .en1_pin = REAR_PIN_SPARE_2_EN,
              .mux_selection = 1,
          },
          {
              .dsel_pin = REAR_PIN_SPARE_3_4_DSEL,
              .en0_pin = REAR_PIN_SPARE_3_EN,
              .en1_pin = REAR_PIN_SPARE_4_EN,
              .mux_selection = 2,
          },
          {
              .dsel_pin = REAR_PIN_SPARE_8_9_DSEL,
              .en0_pin = REAR_PIN_SPARE_8_EN,
              .en1_pin = REAR_PIN_SPARE_9_EN,
              .mux_selection = 6,
          },
      },
  .num_bts7200_channels = 8,
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  { .port = GPIO_PORT_A, .pin = 6 },  //
                  { .port = GPIO_PORT_A, .pin = 5 },  //
                  { .port = GPIO_PORT_A, .pin = 4 },  //
                  { .port = GPIO_PORT_A, .pin = 3 },  //
              },
      },
  .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },  //
  .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },  //
};
