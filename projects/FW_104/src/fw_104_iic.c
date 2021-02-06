#include "fw_104_iic.h"

#include "i2c.h"

#include "misc.h"
#include "status.h"

#define I2C_MESSAGE 0b0010010010100110;

StatusCode write_read_i2c_message(void) {
  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .sda = { .port = GPIO_PORT_B, .pin = 9 },
    .scl = { .port = GPIO_PORT_B, .pin = 8 },
  };

  i2c_init(I2C_PORT_1, &settings);

  // Datasheet for Pedal Board:
  // https://university-of-waterloo-solar-car-team.365.altium.com/designs/69B56210-BCA1-4737-AD11-6778172C6E7C?variant=%5BNo%20Variations%5D#design
  // ADC at https://www.ti.com/lit/ds/symlink/ads1013.pdf
  uint8_t message[2] = { 0b00100100, 0b10100110 };

  // SDA PIN Address = 0b1001010
  // Conversion Register = 0b0000000
  // Configuration Register = 0b0000001
  status_ok_or_return(
      i2c_write_reg(I2C_PORT_1, 0b1001010, 0b0000001, message, SIZEOF_ARRAY(message)));

  uint8_t data[2];
  status_ok_or_return(i2c_read_reg(I2C_PORT_1, 0b1001010, 0b0000000, data, SIZEOF_ARRAY(data)));

  return STATUS_CODE_OK;
}
