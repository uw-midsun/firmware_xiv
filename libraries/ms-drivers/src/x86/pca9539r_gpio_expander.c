#include "pca9539r_gpio_expander.h"

// There's only 256 I2C addresses so it's ok to keep all the settings in memory
#define MAX_I2C_ADDRESSES 256

static Pca9539rGpioSettings s_pin_settings[MAX_I2C_ADDRESSES][NUM_PCA9539R_GPIO_PINS];

StatusCode pca9539r_gpio_init(const I2CAddress i2c_address) {
  // Set each pin to the default settings
  Pca9539rGpioSettings default_settings = {
    .direction = PCA9539R_GPIO_DIR_IN,
    .state = PCA9539R_GPIO_STATE_LOW,
  };
  for (Pca9539rPinAddress i = 0; i < NUM_PCA9539R_GPIO_PINS; i++) {
    s_pin_settings[i2c_address][i] = default_settings;
  }
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_init_pin(const Pca9539rGpioAddress *address,
                                  const Pca9539rGpioSettings *settings) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS || settings->direction >= NUM_PCA9539R_GPIO_DIRS ||
      settings->state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin] = *settings;
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_set_state(const Pca9539rGpioAddress *address,
                                   const Pca9539rGpioState state) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS || state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  s_pin_settings[address->i2c_address][address->pin].state = state;
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_toggle_state(const Pca9539rGpioAddress *address) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (s_pin_settings[address->i2c_address][address->pin].state == PCA9539R_GPIO_STATE_HIGH) {
    s_pin_settings[address->i2c_address][address->pin].state = PCA9539R_GPIO_STATE_LOW;
  } else {
    s_pin_settings[address->i2c_address][address->pin].state = PCA9539R_GPIO_STATE_HIGH;
  }
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_get_state(const Pca9539rGpioAddress *address,
                                   Pca9539rGpioState *input_state) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  *input_state = s_pin_settings[address->i2c_address][address->pin].state;
  return STATUS_CODE_OK;
}
