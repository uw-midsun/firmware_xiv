#include "rear_power_distribution_current_measurement_config.h"

// Definitions of the hardware configs declared in the header

#define REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0 0x74
#define REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1 0x76

// TO-DO ADD ENABLE PINS TO BTS7200 - won't work without

// Note: this is actually entirely for front power distribution
// This will be changed when this module is generified
const RearPowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_HW_CONFIG = {
  .i2c_port = I2C_PORT_2,
  .dsel_i2c_addresses =
      (I2CAddress[]){
          REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,  //
          REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,  //
      },
  .num_dsel_i2c_addresses = 2,
  .bts7200s =
      (RearPowerDistributionBts7200Data[]){
          {
              // LEFT_RIGHT_CAM
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                                     .pin = PCA9539R_PIN_IO0_0 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_LEFT_CAMERA,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_CAMERA,
              .mux_selection = 12,
          },
          {
              // DVR_REAR_DISP
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                                     .pin = PCA9539R_PIN_IO0_6 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_DRIVER_DISPLAY,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_REAR_DISPLAY,
              .mux_selection = 0,
          },
          {
              // L_R_DVR_DISP
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                                     .pin = PCA9539R_PIN_IO1_5 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_LEFT_DRIVER_DISPLAY,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_DRIVER_DISPLAY,
              .mux_selection = 4,
          },
          {
              // FRONT_TURN_LIGHT
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                     .pin = PCA9539R_PIN_IO0_3 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_LEFT_FRONT_TURN_LIGHT,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_RIGHT_FRONT_TURN_LIGHT,
              .mux_selection = 9,
          },
          {
              // MAIN_REAR_PI
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                                     .pin = PCA9539R_PIN_IO1_1 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_MAIN_PI,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_REAR_PI,
              .mux_selection = 1,
          },
          {
              // 5V_SPARE
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                                     .pin = PCA9539R_PIN_IO0_3 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_5V_SPARE_1,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_5V_SPARE_2,
              .mux_selection = 2,
          },
          {
              // SPARE_2_CTR_CONSL
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                     .pin = PCA9539R_PIN_IO0_0 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_2,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_CENTRE_CONSOLE,
              .mux_selection = 7,
          },
          {
              // STR_PDL
              .dsel_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                     .pin = PCA9539R_PIN_IO1_2 },
              .current_0 = REAR_POWER_DISTRIBUTION_CURRENT_STEERING,
              .current_1 = REAR_POWER_DISTRIBUTION_CURRENT_PEDAL,
              .mux_selection = 6,
          } },
  .num_bts7200_channels = 8,
  .bts7040s =
      (RearPowerDistributionBts7040Data[]){
          {
              // MAIN_DISP
              .enable_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                       .pin = PCA9539R_PIN_IO1_4 },
              .current = REAR_POWER_DISTRIBUTION_CURRENT_MAIN_DISPLAY,
              .mux_selection = 13,
          },
          {
              // DAYTIME
              .enable_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                       .pin = PCA9539R_PIN_IO0_5 },
              .current = REAR_POWER_DISTRIBUTION_CURRENT_DAYTIME_RUNNING_LIGHTS,
              .mux_selection = 8,
          },
          {
              // PARKING_BRAKE
              .enable_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                       .pin = PCA9539R_PIN_IO0_7 },
              .current = REAR_POWER_DISTRIBUTION_CURRENT_PARKING_BRAKE,
              .mux_selection = 10,
          },
          {
              // SPARE_1
              .enable_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                       .pin = PCA9539R_PIN_IO0_6 },
              .current = REAR_POWER_DISTRIBUTION_CURRENT_SPARE_1,
              .mux_selection = 11,
          },
          {
              // HORN - actually a BTS7008 but it's the same
              .enable_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                                       .pin = PCA9539R_PIN_IO1_7 },
              .current = REAR_POWER_DISTRIBUTION_CURRENT_HORN,
              .mux_selection = 3,
          },
          {
              // SPEAKER
              .enable_gpio_address = { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                                       .pin = PCA9539R_PIN_IO1_0 },
              .current = REAR_POWER_DISTRIBUTION_CURRENT_SPEAKER,
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
