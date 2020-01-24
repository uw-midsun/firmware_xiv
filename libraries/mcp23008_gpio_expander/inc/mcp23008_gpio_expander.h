#pragma once
// GPIO HAL interface for MCP23008 GPIO expander
#include <stdint.h>

#include "status.h"

#define NUM_MCP_23008_GPIO_PINS 8

// I2C address of the MCP23008 chip
typedef uint8_t Mcp23008I2CAddress;

// GPIO address used to access the pin.
typedef struct {
  Mcp23008I2CAddress i2c_address;
  uint8_t pin;
} Mcp23008GpioAddress;

// For setting the direction of the pin
typedef enum {
  MCP23008_GPIO_DIR_IN = 0,
  MCP23008_GPIO_DIR_OUT,
  NUMMCP_23008_GPIO_DIRS,
} Mcp23008GpioDirection;

// For setting the output value of the pin
typedef enum {
  MCP23008_GPIO_STATE_LOW = 0,
  MCP23008_GPIO_STATE_HIGH,
  NUM_MCP23008_GPIO_STATES,
} Mcp23008GpioState;

typedef struct {
  Mcp23008GpioDirection direction;
  Mcp23008GpioState initial_state;
} Mcp23008GpioSettings;

// Initialize MCP23008 GPIO at this I2C address. Set all pins to default values.
StatusCode mcp23008_gpio_init(const Mcp23008I2CAddress *i2c_address);

// Initialize an MCP23008 GPIO pin by address.
StatusCode mcp23008_gpio_init_pin(const Mcp23008GpioAddress *address, const Mcp23008GpioSettings *settings);

// Set the state of an MCP23008 GPIO pin by address.
StatusCode mcp23008_gpio_set_state(const Mcp23008GpioAddress *address, Mcp23008GpioState state);

// Toggle the output state of the pin.
StatusCode mcp23008_gpio_toggle_state(const Mcp23008GpioAddress *address);

// Get the value of the input register for a pin.
StatusCode mcp23008_gpio_get_state(const Mcp23008GpioAddress *address, Mcp23008GpioState *input_state);
