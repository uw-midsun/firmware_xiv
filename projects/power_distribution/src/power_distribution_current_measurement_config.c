#include "power_distribution_current_measurement_config.h"

// Definitions of the hardware configs declared in the header

#define POWER_DISTRIBUTION_I2C_PORT I2C_PORT_2

#define POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0 0x74
#define POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1 0x76

const PowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG = {
  .i2c_port = POWER_DISTRIBUTION_I2C_PORT,
  .dsel_i2c_addresses =
      (I2CAddress[]){
          POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,  //
          POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,  //
      },
  .num_dsel_i2c_addresses = 2,
  .bts7200s =
      (PowerDistributionBts7200Data[]){
          {
              // LEFT_RIGHT_CAM
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                            .pin = PCA9539R_PIN_IO0_0 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_LEFT_CAMERA,
              .current_1 = POWER_DISTRIBUTION_CURRENT_RIGHT_CAMERA,
              .mux_selection = 12,
          },
          {
              // DVR_REAR_DISP
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                            .pin = PCA9539R_PIN_IO0_6 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_DRIVER_DISPLAY,
              .current_1 = POWER_DISTRIBUTION_CURRENT_REAR_DISPLAY,
              .mux_selection = 0,
          },
          {
              // L_R_DVR_DISP
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                            .pin = PCA9539R_PIN_IO1_5 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_LEFT_DRIVER_DISPLAY,
              .current_1 = POWER_DISTRIBUTION_CURRENT_RIGHT_DRIVER_DISPLAY,
              .mux_selection = 4,
          },
          {
              // FRONT_TURN_LIGHT
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                            .pin = PCA9539R_PIN_IO0_3 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_LEFT_FRONT_TURN_LIGHT,
              .current_1 = POWER_DISTRIBUTION_CURRENT_RIGHT_FRONT_TURN_LIGHT,
              .mux_selection = 9,
          },
          {
              // MAIN_REAR_PI
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                            .pin = PCA9539R_PIN_IO1_1 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_MAIN_PI,
              .current_1 = POWER_DISTRIBUTION_CURRENT_REAR_PI,
              .mux_selection = 1,
          },
          {
              // 5V_SPARE
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                            .pin = PCA9539R_PIN_IO0_3 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_5V_SPARE_1,
              .current_1 = POWER_DISTRIBUTION_CURRENT_5V_SPARE_2,
              .mux_selection = 2,
          },
          {
              // SPARE_2_CTR_CONSL
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                            .pin = PCA9539R_PIN_IO0_0 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_SPARE_2,
              .current_1 = POWER_DISTRIBUTION_CURRENT_CENTRE_CONSOLE,
              .mux_selection = 7,
          },
          {
              // STR_PDL
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                            .pin = PCA9539R_PIN_IO1_2 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_STEERING,
              .current_1 = POWER_DISTRIBUTION_CURRENT_PEDAL,
              .mux_selection = 6,
          } },
  .num_bts7200_channels = 8,
  .bts7040s =
      (PowerDistributionBts7040Data[]){
          {
              // MAIN_DISP
              .current = POWER_DISTRIBUTION_CURRENT_MAIN_DISPLAY,
              .mux_selection = 13,
          },
          {
              // DAYTIME
              .current = POWER_DISTRIBUTION_CURRENT_DAYTIME_RUNNING_LIGHTS,
              .mux_selection = 8,
          },
          {
              // PARKING_BRAKE
              .current = POWER_DISTRIBUTION_CURRENT_PARKING_BRAKE,
              .mux_selection = 10,
          },
          {
              // SPARE_1
              .current = POWER_DISTRIBUTION_CURRENT_SPARE_1,
              .mux_selection = 11,
          },
          {
              // HORN - actually a BTS7008 but it's the same
              .current = POWER_DISTRIBUTION_CURRENT_HORN,
              .mux_selection = 3,
          },
          {
              // SPEAKER
              .current = POWER_DISTRIBUTION_CURRENT_SPEAKER,
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
          POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,  //
          POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,  //
      },
  .num_dsel_i2c_addresses = 2,
  .bts7200s =
      (PowerDistributionBts7200Data[]){
          {
              // Rear camera and placeholder (there's nothing on the second port?)
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                            .pin = PCA9539R_PIN_IO0_0 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_REAR_CAMERA,
              .current_1 = POWER_DISTRIBUTION_CURRENT_PLACEHOLDER_REAR_CAM,
              .mux_selection = 12,
          },
          {
              // Charger interface and lights BPS strobe - assuming on same BTS7200 as L_R_DVR_DISP
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                            .pin = PCA9539R_PIN_IO1_5 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_CHARGER_INTERFACE,
              .current_1 = POWER_DISTRIBUTION_CURRENT_LIGHTS_BPS_STROBE,
              .mux_selection = 4,
          },
          {
              // Left and right rear turn lights
              .dsel_pin = { .i2c_address = POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                            .pin = PCA9539R_PIN_IO0_3 },
              .current_0 = POWER_DISTRIBUTION_CURRENT_LEFT_REAR_TURN_LIGHT,
              .current_1 = POWER_DISTRIBUTION_CURRENT_RIGHT_REAR_TURN_LIGHT,
              .mux_selection = 9,
          } },
  .num_bts7200_channels = 3,
  .bts7040s =
      (PowerDistributionBts7040Data[]){
          {
              // BMS carrier - assuming "infotainment monitor" is MAIN_DISP
              .current = POWER_DISTRIBUTION_CURRENT_BMS_CARRIER,
              .mux_selection = 13,
          },
          {
              // Rear brake light
              .current = POWER_DISTRIBUTION_CURRENT_REAR_BRAKE_LIGHT,
              .mux_selection = 8,
          },
          {
              // Solar sense
              .current = POWER_DISTRIBUTION_CURRENT_SOLAR_SENSE,
              .mux_selection = 10,
          },
          {
              // MCI
              .current = POWER_DISTRIBUTION_CURRENT_MCI,
              .mux_selection = 3,
          } },
  .num_bts7040_channels = 4,
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
