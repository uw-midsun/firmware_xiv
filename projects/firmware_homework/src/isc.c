#include "isc_104.h"

#include "gpio.h"
#include "i2c.h"
#include "log.h"

#define CONVERSION_REGISTER 0x0
#define CONFIGURATION_REGISTER 0x1
#define SDA_ADDRESS 0x48  // From the pinned note in the schematics

StatusCode send_i2c_message(void) {
  // I2C initializations
  static I2CPort port_to_use = I2C_PORT_1;
  const I2CSettings settings = {
    .speed = I2C_SPEED_FAST,
    .sda = { .port = GPIO_PORT_B, 11 },
    .scl = { .port = GPIO_PORT_B, 12 },
  };
  i2c_init(port_to_use, &settings);

  // I2C message is 0b0010010010100110 so split the message as an array
  uint8_t i2c_message[2] = { 0b00100100, 0b10100110 };
  // Write to configuration register
  status_ok_or_return(i2c_write_reg(port_to_use, SDA_ADDRESS, CONFIGURATION_REGISTER, i2c_message,
                                    SIZEOF_ARRAY(i2c_message)));
  // Read from conversion register
  uint8_t data[2] = { 0 };
  status_ok_or_return(
      i2c_read_reg(port_to_use, SDA_ADDRESS, CONVERSION_REGISTER, data, SIZEOF_ARRAY(data)));

  return STATUS_CODE_OK;
}
