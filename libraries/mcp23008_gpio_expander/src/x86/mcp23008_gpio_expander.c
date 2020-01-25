#include "mcp23008_gpio_expander.h"

// There's only 256 I2C addresses so it's ok to keep all the settings in memory.
#define MAX_I2C_ADDRESSES 256

static Mcp23008GpioSettings s_pin_settings[MAX_I2C_ADDRESSES][NUM_MCP23008_GPIO_PINS];

StatusCode mcp23008_gpio_init(const Mcp23008I2CAddress i2c_address) {
  // Set each pin to the default settings
  Mcp23008GpioSettings default_settings = {
    .direction = MCP23008_GPIO_DIR_IN,
    .state = MCP23008_GPIO_STATE_LOW,
  };
  for (Mcp23008PinAddress i = 0; i < NUM_MCP23008_GPIO_PINS; i++) {
    s_pin_settings[i2c_address][i] = default_settings;
  }
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_init_pin(const Mcp23008GpioAddress *address,
                                  const Mcp23008GpioSettings *settings) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS || settings->direction >= NUM_MCP23008_GPIO_DIRS ||
      settings->state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin] = *settings;
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_set_state(const Mcp23008GpioAddress *address, Mcp23008GpioState state) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS || state >= NUM_MCP23008_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin].state = state;
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_toggle_state(const Mcp23008GpioAddress *address) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (s_pin_settings[address->i2c_address][address->pin].state == MCP23008_GPIO_STATE_HIGH) {
    s_pin_settings[address->i2c_address][address->pin].state = MCP23008_GPIO_STATE_LOW;
  } else {
    s_pin_settings[address->i2c_address][address->pin].state = MCP23008_GPIO_STATE_HIGH;
  }
  return STATUS_CODE_OK;
}

StatusCode mcp23008_gpio_get_state(const Mcp23008GpioAddress *address,
                                   Mcp23008GpioState *input_state) {
  if (address->pin >= NUM_MCP23008_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // the gpio module does something strange - check w/ Arshan
  *input_state = s_pin_settings[address->i2c_address][address->pin].state;
  return STATUS_CODE_OK;
}
