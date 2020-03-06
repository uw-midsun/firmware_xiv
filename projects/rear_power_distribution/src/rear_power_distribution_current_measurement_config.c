#include "rear_power_distribution_current_measurement_config.h"

// Definitions of the hardware configs declared in the header

#define REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0 0x74
#define REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1 0x76

// Note: this is actually entirely for front power distribution
// It will be changed when this module is generified
const RearPowerDistributionCurrentHardwareConfig FRONT_POWER_DISTRIBUTION_HW_CONFIG = {
  .i2c_port = I2C_PORT_2,
  .num_bts7200_channels = 6,
  .num_dsel_i2c_addresses = 2,
  .dsel_i2c_addresses =
      {
          REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,  //
          REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,  //
      },
  .bts7200_to_dsel_address =
      (Pca9539rGpioAddress[]){
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_CAMERA] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                .pin = PCA9539R_PIN_IO0_0 },
          [REAR_POWER_DISTRIBUTION_BTS7200_DRIVER_REAR_DISPLAY] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                .pin = PCA9539R_PIN_IO0_6 },
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_DRIVER_DISPLAY] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                .pin = PCA9539R_PIN_IO1_5 },
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_FRONT_TURN_LIGHT] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                .pin = PCA9539R_PIN_IO0_3 },
          [REAR_POWER_DISTRIBUTION_BTS7200_MAIN_REAR_PI] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                .pin = PCA9539R_PIN_IO1_1 },
          [REAR_POWER_DISTRIBUTION_BTS7200_5V_SPARE_1_2] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_0,
                .pin = PCA9539R_PIN_IO0_3 },
          [REAR_POWER_DISTRIBUTION_BTS7200_SPARE_2_CENTRE_CONSOLE] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                .pin = PCA9539R_PIN_IO0_0 },
          [REAR_POWER_DISTRIBUTION_BTS7200_STEERING_PEDAL] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS_1,
                .pin = PCA9539R_PIN_IO1_2 },
      },
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
  .bts7200_to_mux_select =
      (uint8_t[]){
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_CAMERA] = 12,
          [REAR_POWER_DISTRIBUTION_BTS7200_DRIVER_REAR_DISPLAY] = 0,
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_DRIVER_DISPLAY] = 4,
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_FRONT_TURN_LIGHT] = 9,
          [REAR_POWER_DISTRIBUTION_BTS7200_MAIN_REAR_PI] = 1,
          [REAR_POWER_DISTRIBUTION_BTS7200_5V_SPARE_1_2] = 2,
          [REAR_POWER_DISTRIBUTION_BTS7200_SPARE_2_CENTRE_CONSOLE] = 7,
          [REAR_POWER_DISTRIBUTION_BTS7200_STEERING_PEDAL] = 6,
      }
};
