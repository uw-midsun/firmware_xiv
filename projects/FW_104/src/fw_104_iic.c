#include "i2c.h"

#include "fw_104_iic.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define I2C_MESSAGE 0b0010010010100110

StatusCode prv_write_read_i2c_message() {
  // Datasheet for Pedal Board:
  // https://university-of-waterloo-solar-car-team.365.altium.com/designs/69B56210-BCA1-4737-AD11-6778172C6E7C?variant=%5BNo%20Variations%5D#design
  // ADC at https://www.ti.com/lit/ds/symlink/ads1013.pdf

  StatusCode status = NUM_STATUS_CODES;
  uint8_t message[2] = { 0b00100100, 0b10100110 };

  // SDA PIN Address = 0b1001010
  // Conversion Register = 0b0000000
  // Configuration Register = 0b0000001
  status = i2c_write_reg(I2C_PORT_1, 0b1001010, 0b0000001, message, ARRAY_SIZE(message));

  uint8_t data[2];
  status = i2c_read_reg(I2C_PORT_1, 0b1001010, 0b0000000, data, ARRAY_SIZE(data));

  return status;
}
