#include "adt_7476a_fan_controller.h"

#include <stddef.h>

#include "adt_7476a_fan_controller_defs.h"

// need to set interrupt once fan goes out of range
// PWM duty cycle is set from 0-100, in steps of 0.39 (0 - 0xFF)
StatusCode adt7476a_set_speed(I2CPort port, uint8_t speed, uint8_t FAN_PWM_ADDR,
                              uint8_t ADT7476A_I2C_ADDRESS) {
  // need to select duty fan - correct fan duty cycle

  status_ok_or_return(
      i2c_write_reg(port, ADT7476A_I2C_ADDRESS, ADT7476A_PWM_1, &speed, SET_SPEED_NUM_BYTES));
  status_ok_or_return(
      i2c_write_reg(port, ADT7476A_I2C_ADDRESS, ADT7476A_PWM_3, &speed, SET_SPEED_NUM_BYTES));

  return STATUS_CODE_OK;
}

StatusCode get_status(I2CPort port, uint8_t ADT7476A_I2C_ADDRESS, uint8_t register_1_data,
                      uint8_t register_2_data) {
  // read interrupt status register
  status_ok_or_return(i2c_read_reg(port, ADT7476A_I2C_ADDRESS, ADT7476A_INTERRUPT_STATUS_REGISTER_1,
                                   &register_1_data, NUM_BYTES_TO_READ));
  status_ok_or_return(i2c_read_reg(port, ADT7476A_I2C_ADDRESS, ADT7476A_INTERRUPT_STATUS_REGISTER_2,
                                   &register_2_data, NUM_BYTES_TO_READ));

  return STATUS_CODE_OK;
}

StatusCode adt7476a_init(Adt7476aStorage *storage, Adt7476aSettings *settings) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(Adt7476aStorage));

  storage->smbalert_pin = settings->smbalert_pin;
  storage->i2c = settings->i2c;
  storage->i2c_read_addr = settings->i2c_read_addr;
  storage->i2c_write_addr = settings->i2c_write_addr;
  storage->i2c_settings = settings->i2c_settings;
  storage->callback = settings->callback;  // probably will use to check if tachometre reads 0
  storage->callback_context = settings->callback_context;

  static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  status_ok_or_return(i2c_init(storage->i2c, &storage->i2c_settings));

  uint8_t fan_config_data = ADT7476A_MANUAL_MODE_MASK;
  uint8_t smbalert_config_data = ADT7476A_CONFIG_REG_3_MASK;

  // configure pwm to manual mode
  status_ok_or_return(i2c_write_reg(storage->i2c, storage->i2c_write_addr,
                                    ADT7476A_FAN_MODE_REGISTER_1, &fan_config_data,
                                    NUM_BYTES_TO_WRITE));
  status_ok_or_return(i2c_write_reg(storage->i2c, storage->i2c_write_addr,
                                    ADT7476A_FAN_MODE_REGISTER_3, &fan_config_data,
                                    NUM_BYTES_TO_WRITE));

  // set pin 10 to SMBALERT rather than pwm output
  status_ok_or_return(i2c_write_reg(storage->i2c, storage->i2c_write_addr,
                                    ADT7476A_CONFIG_REGISTER_3, &smbalert_config_data,
                                    NUM_BYTES_TO_WRITE));

  gpio_it_register_interrupt(&storage->smbalert_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             storage->callback, storage->callback_context);

  return STATUS_CODE_OK;
}
