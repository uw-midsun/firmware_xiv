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
  .bts7200s =
      (PowerDistributionBts7200Data[]){
          {
              .dsel_pin = FRONT_PIN_LEFT_RIGHT_CAM_DSEL,
              .en0_pin = FRONT_PIN_LEFT_CAMERA_EN, 
              .en1_pin = FRONT_PIN_RIGHT_CAMERA_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_LEFT_CAMERA,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_RIGHT_CAMERA,
              .mux_selection = 12,
          },
          {
              .dsel_pin = FRONT_PIN_DVR_REAR_DISP_DSEL,
              .en0_pin = FRONT_PIN_DVR_DISP_EN,
              .en1_pin = FRONT_PIN_REAR_DISP_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_DRIVER_DISPLAY,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_REAR_DISPLAY,
              .mux_selection = 0,
          },
          {
              .dsel_pin = FRONT_PIN_L_R_DVR_DISP_DSEL,
              .en0_pin = FRONT_PIN_LEFT_DISPLAY_EN,
              .en1_pin = FRONT_PIN_RIGHT_DISPLAY_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_LEFT_DRIVER_DISPLAY,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_RIGHT_DRIVER_DISPLAY,
              .mux_selection = 4,
          },
          {
              .dsel_pin = FRONT_PIN_FRONT_TURN_LIGHT_DSEL,
              .en0_pin = FRONT_PIN_FRONT_LEFT_TURN_EN, 
              .en1_pin = FRONT_PIN_FRONT_RIGHT_TURN_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_LEFT_FRONT_TURN_LIGHT,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_RIGHT_FRONT_TURN_LIGHT,
              .mux_selection = 9,
          },
          {
              .dsel_pin = FRONT_PIN_MAIN_REAR_PI_DSEL,
              .en0_pin = FRONT_PIN_MAIN_PI_B_EN, 
              .en1_pin = FRONT_PIN_REAR_PI_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_MAIN_PI,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_REAR_PI,
              .mux_selection = 1,
          },
          {
              .dsel_pin = FRONT_PIN_5V_SPARE_DSEL,
              .en0_pin = FRONT_PIN_5V_SPARE_1_EN, 
              .en1_pin = FRONT_PIN_5V_SPARE_2_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_5V_SPARE_1,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_5V_SPARE_2,
              .mux_selection = 2,
          },
          {
              .dsel_pin = FRONT_PIN_SPARE_2_CTR_CONSL_DSEL,
              .en0_pin = FRONT_PIN_SPARE_2_EN,
              .en1_pin = FRONT_PIN_CTR_CONSL_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_SPARE_2,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_CENTRE_CONSOLE,
              .mux_selection = 7,
          },
          {
              .dsel_pin = FRONT_PIN_STR_PDL_DSEL,
              .en0_pin = FRONT_PIN_STEERING_EN, 
              .en1_pin = FRONT_PIN_CTR_CONSL_EN,
              .current_0 = FRONT_POWER_DISTRIBUTION_CURRENT_STEERING,
              .current_1 = FRONT_POWER_DISTRIBUTION_CURRENT_PEDAL,
              .mux_selection = 6,
          } },
  .num_bts7200_channels = 8,
  .bts7040s =
      (PowerDistributionBts7040Data[]){
          {
              .current = FRONT_POWER_DISTRIBUTION_CURRENT_MAIN_DISPLAY,
              .mux_selection = 13,
          },
          {
              .current = FRONT_POWER_DISTRIBUTION_CURRENT_DAYTIME_RUNNING_LIGHTS,
              .mux_selection = 8,
          },
          {
              .current = FRONT_POWER_DISTRIBUTION_CURRENT_PARKING_BRAKE,
              .mux_selection = 10,
          },
          {
              .current = FRONT_POWER_DISTRIBUTION_CURRENT_SPARE_1,
              .mux_selection = 11,
          },
          {
              .current = FRONT_POWER_DISTRIBUTION_CURRENT_HORN,
              .mux_selection = 3,
          },
          {
              .current = FRONT_POWER_DISTRIBUTION_CURRENT_SPEAKER,
              .mux_selection = 5,
          } },
  .num_bts7040_channels = 6,
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
          .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },  //
          .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },  //
      },
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
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_SOLAR_SENSE,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_TELEMETRY,
              .mux_selection = 7,
          },
          {
              .dsel_pin = REAR_PIN_STROBE_CTR_BRK_DSEL,
              .en0_pin = REAR_PIN_STROBE_LIGHT_EN, 
              .en1_pin = REAR_PIN_CENTER_BRAKE_LIGHT_EN,
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_STROBE,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_CENTRE_BRAKE_LIGHT,
              .mux_selection = 9,
          },
          {
              .dsel_pin = REAR_PIN_REAR_TURN_LIGHT_DSEL,
              .en0_pin = REAR_PIN_REAR_LEFT_TURN_EN,
              .en1_pin = REAR_PIN_REAR_RIGHT_TURN_EN,
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_LEFT_REAR_TURN_LIGHT,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_REAR_TURN_LIGHT,
              .mux_selection = 0,
          },
          {
              .dsel_pin = REAR_PIN_REAR_BRAKE_LIGHT_DSEL,
              .en0_pin = REAR_PIN_REAR_LEFT_BRAKEL_EN, 
              .en1_pin = REAR_PIN_REAR_RIGHT_BRAKEL_EN,
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_LEFT_BRAKE_LIGHT,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_BRAKE_LIGHT,
              .mux_selection = 4,
          },
          {
              .dsel_pin = REAR_PIN_CAM_SPARE_10_DSEL,
              .en0_pin = REAR_PIN_REAR_CAMERA_EN, 
              .en1_pin = REAR_PIN_SPARE_10_EN,
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_REAR_CAMERA,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_10,
              .mux_selection = 12,
          },
          {
              .dsel_pin = REAR_PIN_SPARE_1_2_DSEL,
              .en0_pin = REAR_PIN_SPARE_1_EN,
              .en1_pin = REAR_PIN_SPARE_2_EN,
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_1,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_2,
              .mux_selection = 1,
          },
          {
              .dsel_pin = REAR_PIN_SPARE_3_4_DSEL,
              .en0_pin = REAR_PIN_SPARE_3_EN, 
              .en1_pin = REAR_PIN_SPARE_4_EN,
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_3,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_4,
              .mux_selection = 2,
          },
          {
              .dsel_pin = REAR_PIN_SPARE_8_9_DSEL,
              .en0_pin = REAR_PIN_SPARE_8_EN, 
              .en1_pin = REAR_PIN_SPARE_9_EN,
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_8,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_9,
              .mux_selection = 6,
          },
      },
  .num_bts7200_channels = 8,
  .bts7040s =
      (PowerDistributionBts7040Data[]){
          {
              .current = REAR_POWER_DISTRIBUTION_CURRENT_BMS_CARRIER,
              .mux_selection = 8,
          },
          {
              .current = REAR_POWER_DISTRIBUTION_CURRENT_MCI,
              .mux_selection = 13,
          },
          {
              .current = REAR_POWER_DISTRIBUTION_CURRENT_CHARGER,
              .mux_selection = 11,
          },
          {
              .current = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_5,
              .mux_selection = 10,
          },
          {
              .current = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_6,
              .mux_selection = 3,
          },
          {
              .current = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_7,
              .mux_selection = 5,
          },
      },
  .num_bts7040_channels = 6,
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
          .mux_output_pin = { .port = GPIO_PORT_A, .pin = 7 },  //
          .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 2 },  //
      },
};
