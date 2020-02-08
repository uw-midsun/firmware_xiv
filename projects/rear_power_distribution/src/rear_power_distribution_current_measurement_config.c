#include "rear_power_distribution_current_measurement_config.h"

// Definitions of the hardware configs declared in the header

const RearPowerDistributionCurrentHardwareConfig REAR_POWER_DISTRIBUTION_HW_CONFIG = {
  .num_bts7200_channels = 4,
  .dsel_i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS,
  .bts7200_to_dsel_address =
      (Mcp23008GpioAddress[]){
          [REAR_POWER_DISTRIBUTION_BTS7200_CTR_BRK_STROBE] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS, .pin = 1 },
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_BRK_LIGHT] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS, .pin = 2 },
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_TURN] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS, .pin = 3 },
          [REAR_POWER_DISTRIBUTION_BTS7200_SOLAR_TELEMETRY] =
              { .i2c_address = REAR_POWER_DISTRIBUTION_DSEL_I2C_ADDRESS, .pin = 7 },
      },
  .mux_address =
      {
          .sel_pins =
              {
                  { .port = GPIO_PORT_A, .pin = 6 },  //
                  { .port = GPIO_PORT_A, .pin = 5 },  //
                  { .port = GPIO_PORT_A, .pin = 4 },  //
              },
          .mux_output_pin = { .port = GPIO_PORT_B, .pin = 0 },  //
      },
  .bts7200_to_mux_select =
      (uint8_t[]){
          [REAR_POWER_DISTRIBUTION_BTS7200_CTR_BRK_STROBE] = 2,
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_BRK_LIGHT] = 3,
          [REAR_POWER_DISTRIBUTION_BTS7200_LEFT_RIGHT_TURN] = 0,
          [REAR_POWER_DISTRIBUTION_BTS7200_SOLAR_TELEMETRY] = 7,
      }
};
