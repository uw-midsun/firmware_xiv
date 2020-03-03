#include "mcp23008_gpio_expander.h"

#include <stdbool.h>
#include "mcp23008_gpio_expander_defs.h"

StatusCode mcp23008_gpio_init(const I2CAddress i2c_address) {
  // this function is implemented only on x86
  return STATUS_CODE_OK;
}

static void prv_set_reg_bit(const Mcp23008GpioAddress *address, uint8_t reg, bool val) {
  uint8_t data = 0;
  i2c_read_reg(address->i2c_port, address->i2c_address, reg, &data, 1);
  if (val) {
    data |= 1 << address->pin;
  } else {
    data &= ~(1 << address->pin);
  }
  i2c_write_reg(address->i2c_port, address->i2c_address, reg, &data, 1);
}

StatusCode mcp23008_gpio_init_pin(const Mcp23008GpioAddress *address,
                                  const Mcp23008GpioSettings *settings) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS || settings->direction >= NUM_MCP23008_GPIO_DIRS ||
      settings->state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the IODIR bit
  prv_set_reg_bit(address, IODIR, settings->direction == MCP23008_GPIO_DIR_IN);

  if (settings->direction == MCP23008_GPIO_DIR_OUT) {
    mcp23008_gpio_set_state(address, settings->state);
  }

  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_set_state(const Mcp23008GpioAddress *address,
                                   const Mcp23008GpioState state) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS || state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_set_reg_bit(address, GPIO, state == MCP23008_GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_toggle_state(const Mcp23008GpioAddress *address) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // optimization: instead of using set_state and get_state, we read only once
  uint8_t gpio_data = 0;
  i2c_read_reg(address->i2c_port, address->i2c_address, GPIO, &gpio_data, 1);
  gpio_data ^= 1 << address->pin;  // toggle the relevant bit
  i2c_write_reg(address->i2c_port, address->i2c_address, GPIO, &gpio_data, 1);

  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_get_state(const Mcp23008GpioAddress *address,
                                   Mcp23008GpioState *input_state) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t gpio_data = 0;
  i2c_read_reg(address->i2c_port, address->i2c_address, GPIO, &gpio_data, 1);

  // Read the |address->pin|th bit
  *input_state =
      ((gpio_data & (1 << address->pin)) == 0) ? MCP23008_GPIO_STATE_LOW : MCP23008_GPIO_STATE_HIGH;
  return STATUS_CODE_OK;
}
