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
StatusCode adt7476a_set_speed(I2CPort port, uint8_t speed_percent, AdtPwmPort pwm_port,
                              uint8_t adt7476a_i2c_address) {
  // check for out of range conditions.
  if (speed_percent > 100) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  // determine which PWM output to change
  uint8_t real_speed = (speed_percent / 0.39);
  if (pwm_port == ADT_PWM_PORT_1) {
    uint8_t adt7476a_speed_register[] = { ADT7476A_PWM_1, real_speed };

    status_ok_or_return(i2c_write(port, adt7476a_i2c_address, adt7476a_speed_register,
                                  SIZEOF_ARRAY(adt7476a_speed_register)));

  } else if (pwm_port == ADT_PWM_PORT_2) {
    uint8_t adt7476a_speed_register[] = { ADT7476A_PWM_3, real_speed };

    status_ok_or_return(i2c_write(port, adt7476a_i2c_address, adt7476a_speed_register,
                                  SIZEOF_ARRAY(adt7476a_speed_register)));

  } else if (pwm_port == ADT_PWM_PORT_3) {
    return STATUS_CODE_UNIMPLEMENTED;
  } else {
    return STATUS_CODE_INVALID_ARGS;
  }

  return STATUS_CODE_OK;
}

StatusCode adt7476a_get_status(I2CPort port, uint8_t adt7476a_i2c_read_address,
                               uint8_t *register_1_data, uint8_t *register_2_data) {
  // read interrupt status register
  status_ok_or_return(i2c_read_reg(port, adt7476a_i2c_read_address,
                                   ADT7476A_INTERRUPT_STATUS_REGISTER_1, register_1_data,
                                   ADT7476A_REG_SIZE));
  status_ok_or_return(i2c_read_reg(port, adt7476a_i2c_read_address,
                                   ADT7476A_INTERRUPT_STATUS_REGISTER_2, register_2_data,
                                   ADT7476A_REG_SIZE));

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
  storage->i2c = settings->i2c;

  static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  status_ok_or_return(i2c_init(settings->i2c, &settings->i2c_settings));

  uint8_t fan_config_data_register_1[] = { ADT7476A_FAN_MODE_REGISTER_1,
                                           ADT7476A_MANUAL_MODE_MASK };
  uint8_t fan_config_data_register_3[] = { ADT7476A_FAN_MODE_REGISTER_3,
                                           ADT7476A_MANUAL_MODE_MASK };
  uint8_t strt_config_data[] = { ADT7476A_CONFIG_REGISTER_1, ADT7476A_CONFIG_REG_1_MASK };
  uint8_t smbalert_config_data[] = { ADT7476A_CONFIG_REGISTER_3, ADT7476A_CONFIG_REG_3_MASK };

  // set STRT bit to on
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, strt_config_data,
                                SIZEOF_ARRAY(strt_config_data)));

  // configure pwm to manual mode
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, fan_config_data_register_1,
                                SIZEOF_ARRAY(fan_config_data_register_1)));
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, fan_config_data_register_3,
                                SIZEOF_ARRAY(fan_config_data_register_3)));

  // set pin 10 to SMBALERT rather than pwm output
  status_ok_or_return(i2c_write(settings->i2c, settings->i2c_write_addr, smbalert_config_data,
                                SIZEOF_ARRAY(smbalert_config_data)));

  gpio_it_register_interrupt(&storage->smbalert_pin, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             storage->callback, storage->callback_context);

  return STATUS_CODE_OK;
}
