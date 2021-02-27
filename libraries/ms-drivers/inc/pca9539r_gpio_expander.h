#pragma once
// GPIO HAL interface for the PCA9539RPW/Q900J GPIO expander.
// Requires I2C , GPIO_IT to be initialized.
// Note: we don't check the validity of the I2C address, and it can only be used on one
// I2C port on one board.
#include "gpio_it.h"
#include "i2c.h"

// Addresses of the 16 pins.
typedef enum {
  PCA9539R_PIN_IO0_0 = 0,
  PCA9539R_PIN_IO0_1,
  PCA9539R_PIN_IO0_2,
  PCA9539R_PIN_IO0_3,
  PCA9539R_PIN_IO0_4,
  PCA9539R_PIN_IO0_5,
  PCA9539R_PIN_IO0_6,
  PCA9539R_PIN_IO0_7,
  PCA9539R_PIN_IO1_0,
  PCA9539R_PIN_IO1_1,
  PCA9539R_PIN_IO1_2,
  PCA9539R_PIN_IO1_3,
  PCA9539R_PIN_IO1_4,
  PCA9539R_PIN_IO1_5,
  PCA9539R_PIN_IO1_6,
  PCA9539R_PIN_IO1_7,
  NUM_PCA9539R_GPIO_PINS,
} Pca9539rPinAddress;

// GPIO address used to access the pin.
typedef struct {
  I2CAddress i2c_address;
  Pca9539rPinAddress pin;
} Pca9539rGpioAddress;

// For setting the direction of the pin.
typedef enum {
  PCA9539R_GPIO_DIR_IN = 0,
  PCA9539R_GPIO_DIR_OUT,
  NUM_PCA9539R_GPIO_DIRS,
} Pca9539rGpioDirection;

// For setting the output value of the pin.
typedef enum {
  PCA9539R_GPIO_STATE_LOW = 0,
  PCA9539R_GPIO_STATE_HIGH,
  NUM_PCA9539R_GPIO_STATES,
} Pca9539rGpioState;

typedef struct {
  Pca9539rGpioDirection direction;
  Pca9539rGpioState state;
} Pca9539rGpioSettings;

typedef GpioItCallback Pca9539rInterruptCallback;

// Initialize PCA9539R GPIO at this I2C port and address.
StatusCode pca9539r_gpio_init(const I2CPort i2c_port, const I2CAddress i2c_address);

// Initialize an PCA9539R GPIO pin by address.
StatusCode pca9539r_gpio_init_pin(const Pca9539rGpioAddress *address,
                                  const Pca9539rGpioSettings *settings);

// Set the state of an PCA9539R GPIO pin by address.
StatusCode pca9539r_gpio_set_state(const Pca9539rGpioAddress *address,
                                   const Pca9539rGpioState state);

// Toggle the output state of the pin.
StatusCode pca9539r_gpio_toggle_state(const Pca9539rGpioAddress *address);

// Get the value of the input register for a pin.
StatusCode pca9539r_gpio_get_state(const Pca9539rGpioAddress *address,
                                   Pca9539rGpioState *input_state);

// Sets up a gpio interrupt on the given pin and calls the callback when it goes low

// Callback MUST CALL gpio_get_state on the (possible) toggled pin(s),
// This must be done as gpio_get_state is the only way to clear the interrupt
StatusCode pca9539r_gpio_subscribe_interrupts(const GpioAddress *interrupt_pin,
                                              Pca9539rInterruptCallback callback, void *context);
