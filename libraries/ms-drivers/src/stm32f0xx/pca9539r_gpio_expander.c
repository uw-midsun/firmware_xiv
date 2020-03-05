#include "pca9539r_gpio_expander.h"

#include <stdbool.h>
#include "i2c_driver_defs.h"
#include "pca9539r_gpio_expander_defs.h"

static bool prv_is_port_0(const Pca9539rPinAddress pin) {
  return pin < PCA9539R_PIN_IO1_0;
}

static uint8_t prv_select_reg(const Pca9539rPinAddress pin, uint8_t reg0, uint8_t reg1) {
  return prv_is_port_0(pin) ? reg0 : reg1;
}

// get the bit representing this pin in an 8-bit register
static uint8_t prv_pin_bit(const Pca9539rPinAddress pin) {
  return prv_is_port_0(pin) ? pin : pin - PCA9539R_PIN_IO1_0;
}

StatusCode pca9539r_gpio_init(const I2CAddress i2c_address) {
  // implemented only on x86
  return STATUS_CODE_OK;
}

static void prv_set_reg_bit(uint8_t i2c_address, uint8_t reg0, uint8_t reg1,
                            const Pca9539rPinAddress pin, bool val) {
  uint8_t reg = prv_select_reg(pin, reg0, reg1);
  uint8_t bit = prv_pin_bit(pin);

  uint8_t data = 0;
  i2c_read_reg(I2C_PORT, i2c_address, reg, &data, 1);
  if (val) {
    data |= 1 << bit;
  } else {
    data &= ~(1 << bit);
  }
  i2c_write_reg(I2C_PORT, i2c_address, reg, &data, 1);
}

StatusCode pca9539r_gpio_init_pin(const Pca9539rGpioAddress *address,
                                  const Pca9539rGpioSettings *settings) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS || settings->direction >= NUM_PCA9539R_GPIO_DIRS ||
      settings->state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the IODIR bit; 0 = output, 1 = input
  prv_set_reg_bit(address->i2c_address, IODIR0, IODIR1, address->pin,
                  settings->direction == PCA9539R_GPIO_DIR_IN);

  if (settings->direction == PCA9539R_GPIO_DIR_OUT) {
    prv_set_reg_bit(address->i2c_address, OUTPUT0, OUTPUT1, address->pin, settings->state);
  }

  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_set_state(const Pca9539rGpioAddress *address,
                                   const Pca9539rGpioState state) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS || state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_set_reg_bit(address->i2c_address, OUTPUT0, OUTPUT1, address->pin,
                  state == PCA9539R_GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_toggle_state(const Pca9539rGpioAddress *address) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // optimization: instead of using set_state and get_state, we read only once
  uint8_t gpio_data = 0;
  i2c_read_reg(I2C_PORT, address->i2c_address, prv_select_reg(address->pin, INPUT0, INPUT1),
               &gpio_data, 1);
  gpio_data ^= 1 << prv_pin_bit(address->pin);
  i2c_write_reg(I2C_PORT, address->i2c_address, prv_select_reg(address->pin, OUTPUT0, OUTPUT1),
                &gpio_data, 1);

  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_get_state(const Pca9539rGpioAddress *address,
                                   Pca9539rGpioState *input_state) {
  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t gpio_data = 0;
  i2c_read_reg(I2C_PORT, address->i2c_address, prv_select_reg(address->pin, INPUT0, INPUT1),
               &gpio_data, 1);

  // Read the |bit|th bit
  uint8_t bit = prv_pin_bit(address->pin);
  *input_state =
      ((gpio_data & (1 << bit)) == 0) ? PCA9539R_GPIO_STATE_LOW : PCA9539R_GPIO_STATE_HIGH;
  return STATUS_CODE_OK;
}
