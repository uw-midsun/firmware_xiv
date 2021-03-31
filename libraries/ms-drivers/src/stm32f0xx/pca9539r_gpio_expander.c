#include "pca9539r_gpio_expander.h"

#include <stdbool.h>

#include "pca9539r_gpio_expander_defs.h"

// The I2C port used for all operations - won't change on each board
static I2CPort s_i2c_port = NUM_I2C_PORTS;

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

StatusCode pca9539r_gpio_init(const I2CPort i2c_port, const I2CAddress i2c_address) {
  if (s_i2c_port == NUM_I2C_PORTS) {
    s_i2c_port = i2c_port;
  }
  return STATUS_CODE_OK;
}

static void prv_read_reg(const Pca9539rGpioAddress *address, uint8_t reg0, uint8_t reg1,
                         uint8_t *rx_data) {
  uint8_t reg = prv_select_reg(address->pin, reg0, reg1);
  i2c_read_reg(s_i2c_port, address->i2c_address, reg, rx_data, 1);
}

static void prv_write_reg(const Pca9539rGpioAddress *address, uint8_t reg0, uint8_t reg1,
                          uint8_t tx_data) {
  // PCA9539R expects the register ("command byte") as just a data byte (see figs 10, 11)
  uint8_t reg = prv_select_reg(address->pin, reg0, reg1);
  uint8_t data[] = { reg, tx_data };
  i2c_write(s_i2c_port, address->i2c_address, data, SIZEOF_ARRAY(data));
}

static void prv_set_reg_bit(const Pca9539rGpioAddress *address, uint8_t reg0, uint8_t reg1,
                            bool val) {
  uint8_t bit = prv_pin_bit(address->pin);
  uint8_t data = 0;
  prv_read_reg(address, reg0, reg1, &data);
  if (val) {
    data |= 1 << bit;
  } else {
    data &= ~(1 << bit);
  }
  prv_write_reg(address, reg0, reg1, data);
}

StatusCode pca9539r_gpio_init_pin(const Pca9539rGpioAddress *address,
                                  const Pca9539rGpioSettings *settings) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS || settings->direction >= NUM_PCA9539R_GPIO_DIRS ||
      settings->state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // Set the IODIR bit; 0 = output, 1 = input
  prv_set_reg_bit(address, IODIR0, IODIR1, settings->direction == PCA9539R_GPIO_DIR_IN);

  if (settings->direction == PCA9539R_GPIO_DIR_OUT) {
    prv_set_reg_bit(address, OUTPUT0, OUTPUT1, settings->state);
  }

  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_set_state(const Pca9539rGpioAddress *address,
                                   const Pca9539rGpioState state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS || state >= NUM_PCA9539R_GPIO_STATES) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  prv_set_reg_bit(address, OUTPUT0, OUTPUT1, state == PCA9539R_GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_toggle_state(const Pca9539rGpioAddress *address) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // optimization: instead of using set_state and get_state, we read only once
  uint8_t gpio_data = 0;
  prv_read_reg(address, INPUT0, INPUT1, &gpio_data);
  gpio_data ^= 1 << prv_pin_bit(address->pin);
  prv_write_reg(address, OUTPUT0, OUTPUT1, gpio_data);

  return STATUS_CODE_OK;
}

StatusCode pca9539r_gpio_get_state(const Pca9539rGpioAddress *address,
                                   Pca9539rGpioState *input_state) {
  if (s_i2c_port >= NUM_I2C_PORTS) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  if (address->pin >= NUM_PCA9539R_GPIO_PINS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  uint8_t gpio_data = 0;
  prv_read_reg(address, INPUT0, INPUT1, &gpio_data);

  // Read the |bit|th bit
  uint8_t bit = prv_pin_bit(address->pin);
  *input_state =
      ((gpio_data & (1 << bit)) == 0) ? PCA9539R_GPIO_STATE_LOW : PCA9539R_GPIO_STATE_HIGH;
  return STATUS_CODE_OK;
}
