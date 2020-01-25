#include "mcp23008_gpio_expander.h"

#include <stdbool.h>
#include "i2c.h"
#include "mcp23008_gpio_expander_defs.h"

StatusCode mcp23008_gpio_init(const Mcp23008I2CAddress i2c_address) {
  // i2c_address only used in x86 - should this even have any implementation?
  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,    //
    .sda = CONFIG_PIN_I2C_SDA,  //
    .scl = CONFIG_PIN_I2C_SCL,  //
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

StatusCode mcp23008_gpio_init_pin(const Mcp23008GpioAddress *address,
                                  const Mcp23008GpioSettings *settings) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS || settings->direction >= NUM_MCP23008_GPIO_DIRS ||
      settings->state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the IODIR bit
  prv_set_reg_bit(address->i2c_address, IODIR, address->pin,
                  settings->direction == MCP23008_GPIO_DIR_IN);

  mcp23008_set_state(address, settings->state);

  return STATUS_CODE_OK;
}

StatusCode mcp23008_set_state(const Mcp23008GpioAddress *address, const Mcp23008GpioState state) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS || state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the GPIO bit
  prv_set_reg_bit(address->i2c_address, GPIO, address->pin, state == MCP23008_GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_toggle_state(const Mcp23008GpioAddress *address) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // optimization: instead of using set_state and get_state, we read only once
  uint8_t gpio_data = 0;
  i2c_read_reg(I2C_PORT, address->i2c_address, GPIO, &gpio_data, 1);
  gpio_data ^= 1 << address->pin;  // toggle the relevant bit
  i2c_write_reg(I2C_PORT, address->i2c_address, GPIO, &gpio_data, 1);

  return STATUS_CODE_OK;
}

StatusCode mcp23008_get_state(const Mcp23008GpioAddress *address, Mcp23008GpioState *input_state) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Read from GPIO
  uint8_t gpio_data = 0;
  i2c_read_reg(I2C_PORT, address->i2c_address, GPIO, &gpio_data, 1);

  // Read the |address->pin|th bit
  input_state =
      ((gpio_data & (1 << address->pin)) == 0) ? MCP23008_GPIO_STATE_LOW : MCP23008_GPIO_STATE_HIGH;
  return STATUS_CODE_OK;
}
