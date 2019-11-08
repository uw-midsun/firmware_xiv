#pragma once
// GPIO HAL Interface
#include <stdint.h>

#include "gpio_mcu.h"
#include "status.h"

// GPIO address to be used to change that pin's settings. Both port and pin are
// zero indexed.
typedef struct GpioAddress {
  uint8_t port;
  uint8_t pin;
} GpioAddress;

// For setting the direction of the pin
typedef enum {
  GPIO_DIR_IN = 0,
  GPIO_DIR_OUT,
  GPIO_DIR_OUT_OD,  // Output open-drain
  NUM_GPIO_DIRS,
} GpioDir;

// For setting the output value of the pin
typedef enum {
  GPIO_STATE_LOW = 0,
  GPIO_STATE_HIGH,
  NUM_GPIO_STATES,
} GpioState;

// For setting the internal pull-up/pull-down resistor
typedef enum {
  GPIO_RES_NONE = 0,
  GPIO_RES_PULLUP,
  GPIO_RES_PULLDOWN,
  NUM_GPIO_RESES,
} GpioRes;

// For setting the alternate function on the pin. The specific meaning of each
// depends on the architecture and platform refer to the datasheet for the
// stm32f0xx for specifics. Not implemented on x86.
typedef enum {
  GPIO_ALTFN_NONE = 0,
  GPIO_ALTFN_0,
  GPIO_ALTFN_1,
  GPIO_ALTFN_2,
  GPIO_ALTFN_3,
  GPIO_ALTFN_4,
  GPIO_ALTFN_5,
  GPIO_ALTFN_6,
  GPIO_ALTFN_7,
  GPIO_ALTFN_ANALOG,
  NUM_GPIO_ALTFNS,
} GpioAltFn;

// GPIO settings for setting the value of a pin
typedef struct GpioSettings {
  GpioDir direction;
  GpioState state;
  GpioRes resistor;
  GpioAltFn alt_function;
} GpioSettings;

// Initializes GPIO globally by setting all pins to their default state. ONLY
// CALL ONCE or it will deinit all current settings. Change setting by calling
// gpio_init_pin.
StatusCode gpio_init(void);

// Initializes a GPIO pin by address.
StatusCode gpio_init_pin(const GpioAddress *address, const GpioSettings *settings);

// Set the pin state by address.
StatusCode gpio_set_state(const GpioAddress *address, GpioState state);

// Toggles the output state of the pin.
StatusCode gpio_toggle_state(const GpioAddress *address);

// Gets the value of the input register for a pin and assigns it to the state
// that is passed in.
StatusCode gpio_get_state(const GpioAddress *address, GpioState *input_state);
