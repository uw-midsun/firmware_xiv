#include <math.h>
#include <stddef.h>
#include <string.h>

#include "adt7476a_fan_controller.h"
#include "adt7476a_fan_controller_defs.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "soft_timer.h"

// need to set interrupt once fan goes out of range
// PWM duty cycle is set from 0-100, in steps of 0.39 (0x00 - 0xFF)
// accepts number between 0-100, converts into into range of 0x00 - 0xFF
StatusCode adt7476a_set_speed(I2CPort port, uint8_t speed_percent, AdtFanGroup fan_group,
                              uint8_t adt7476a_i2c_address) {
  // determine which PWM output to change
  uint8_t real_speed = floor(speed_percent / 0.39);
  if (fan_group == ADT_FAN_GROUP_1) {
    status_ok_or_return(i2c_write_reg(port, adt7476a_i2c_address, ADT7476A_PWM_1, &speed_percent,
                                      SET_SPEED_NUM_BYTES));
  } else {
    status_ok_or_return(i2c_write_reg(port, adt7476a_i2c_address, ADT7476A_PWM_3, &speed_percent,
                                      SET_SPEED_NUM_BYTES));
  }

  return STATUS_CODE_OK;
}

StatusCode adt7476a_get_status(I2CPort port, uint8_t adt7476a_i2c_address, uint8_t *register_1_data,
                               uint8_t *register_2_data) {
  // read interrupt status register
  status_ok_or_return(i2c_read_reg(port, adt7476a_i2c_address, ADT7476A_INTERRUPT_STATUS_REGISTER_1,
                                   register_1_data, ADT7476A_REG_SIZE));
  status_ok_or_return(i2c_read_reg(port, adt7476a_i2c_address, ADT7476A_INTERRUPT_STATUS_REGISTER_2,
                                   register_2_data, ADT7476A_REG_SIZE));

  return STATUS_CODE_OK;
}

StatusCode adt7476a_init(Adt7476aStorage *storage, Adt7476aSettings *settings) {
  if (storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(Adt7476aStorage));

  storage->smbalert_pin = settings->smbalert_pin;
  storage->callback = settings->callback;  // called when tachometer goes out of range
  storage->callback_context = settings->callback_context;

  static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  status_ok_or_return(i2c_init(settings->i2c, &settings->i2c_settings));

  uint8_t fan_config_data = ADT7476A_MANUAL_MODE_MASK;
  uint8_t smbalert_config_data = ADT7476A_CONFIG_REG_3_MASK;

  // configure pwm to manual mode
  status_ok_or_return(i2c_write_reg(settings->i2c, settings->i2c_write_addr,
                                    ADT7476A_FAN_MODE_REGISTER_1, &fan_config_data,
                                    ADT7476A_REG_SIZE));
  status_ok_or_return(i2c_write_reg(settings->i2c, settings->i2c_write_addr,
                                    ADT7476A_FAN_MODE_REGISTER_3, &fan_config_data,
                                    ADT7476A_REG_SIZE));

  // set pin 10 to SMBALERT rather than pwm output
  status_ok_or_return(i2c_write_reg(settings->i2c, settings->i2c_write_addr,
                                    ADT7476A_CONFIG_REGISTER_3, &smbalert_config_data,
                                    ADT7476A_REG_SIZE));

  gpio_it_register_interrupt(&storage->smbalert_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             storage->callback, storage->callback_context);

  return STATUS_CODE_OK;
}
