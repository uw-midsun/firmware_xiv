#include "log.h"
#include "gpio.h"
#include "i2c.h"

#define CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

#define MCP_I2C_ADDR 0x22
#define IODIR 0x00

int main(void) {
  gpio_init();

  I2CSettings settings = {
    .speed = I2C_SPEED_FAST,                   //
    .sda = CONFIG_PIN_I2C_SDA,  //
    .scl = CONFIG_PIN_I2C_SCL,  //
  };
  i2c_init(I2C_PORT_2, &settings);
  // try reading form reg 0 and see what we get
  // write 0x4 to IODIR
  uint8_t data = 0x4;
  i2c_write_reg(I2C_PORT_2, MCP_I2C_ADDR, IODIR, &data, 1);
  // read from IODIR to make sure it's 0x4
  uint8_t read_data = 0;
  i2c_read_reg(I2C_PORT_2, MCP_I2C_ADDR, IODIR, &read_data, 1);
  LOG_DEBUG("wrote: %d\nread: %d\n", data, read_data);
  return 0;
}
