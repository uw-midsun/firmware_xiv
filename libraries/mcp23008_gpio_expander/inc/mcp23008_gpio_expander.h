#pragma once
// GPIO interface for MCP23008 GPIO expander
#include <stdint.h>

#include "status.h"

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

// For setting the polarity of the pin. If set to MCP23008_GPIO_POLARITY_INVERTED, the output
// GPIO value will be the opposite of the set state.
typedef enum {
  MCP23008_GPIO_POLARITY_SAME = 0,
  MCP23008_GPIO_POLARITY_INVERTED,
  NUM_MCP23008_GPIO_POLARITIES,
} Mcp23008GpioPolarity;

// For setting whether to use the internal pull-up resistor. MCP23008 appears not to have an
// internal pull-down resistor.
typedef enum {
  MCP23008_GPIO_RESISTOR_NONE = 0,
  MCP23008_GPIO_RESISTOR_PULLUP,
  NUM_MCP23008_GPIO_RESISTORS,
} Mcp23008GpioResistor;

typedef struct {
  Mcp23008GpioDirection direction;
  Mcp23008GpioState initial_state;
  Mcp23008GpioPolarity polarity;
  Mcp23008GpioResistor resistor;
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
