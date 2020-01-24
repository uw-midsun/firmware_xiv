#include "mcp23008_gpio_expander.h"

#include <stdbool.h>
#include "i2c.h"

#define I2C_PORT I2C_PORT_2

#define CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

#define IODIR 0x00 // direction, 1 input 0 output
#define IPOL 0x01 // polarity, corresponding GPIO bit will show inverted value on the pin
#define GPINTEN 0x02 // interrupt on change 1 enable 0 disable
#define DEFVAL 0x03 //  default compare value for interrupt, the opposite value on the pin will cause interrupt to occur
#define INTCON 0x04 // interrupt controll, interrupt-on-change vs interrupt-on-previous-val

// IO configuration register
// bit 5 (SEQOP): sequential operation register, 0 if enabled. Address pointer increments. (apparently good for polling)
// bit 4 (DISSLW): slew rate control 0 enable 1 disable
// bit 3 (HAEN): HW addr bit enable (always enabled for this model)
// bit 2 (ODR): INT pin has open drain ouptut
// bit 1 (INTPOL): polarity for INT ouput pin
#define IOCON 0x05 

#define GPPU 0x06 // pull-up-resistor 1 pulled up
#define INTF 0x07 // Interrupt flag, 1 means that pin caused interrupt
#define INTCAP 0x08 // captures the GPIO PORT value when interrupt occurs
#define GPIO 0x09 // read GPIO from here, write modifies the Output Latch
#define OLAT 0x0A // 'read' reads OLAT, write modifies the pins configured as output

StatusCode mcp23008_gpio_init(const Mcp23008I2CAddress *i2c_address) {
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST, //
    .sda = CONFIG_PIN_I2C_SDA, //
    .scl = CONFIG_PIN_I2C_SCL, //
  };
  // is this ok to do here?
  return i2c_init(I2C_PORT, &i2c_settings);
}

void prv_set_reg_bit(uint8_t i2c_address, uint8_t reg, uint8_t bit, bool val) {
  uint8_t data = 0;
  i2c_read_reg(I2C_PORT, i2c_address, reg, &data, 1);
  if (val) {
    data |= 1 << bit;
  } else {
    data &= ~(1 << bit);
  }
  i2c_write_reg(I2C_PORT, i2c_address, reg, &data, 1);
}

StatusCode mcp23008_gpio_init_pin(const Mcp23008GpioAddress *address, const Mcp23008GpioSettings *settings) {
  if (address->pin >= NUM_MCP_23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  
  // Set the IODIR bit
  prv_set_reg_bit(address->i2c_address, IODIR, address->pin, settings->direction == MCP23008_GPIO_DIR_IN);
  
  // Set the GPIO bit
  prv_set_reg_bit(address->i2c_address, GPIO, address->pin, settings->initial_state == MCP23008_GPIO_STATE_HIGH);
  
  return STATUS_CODE_OK;
}
