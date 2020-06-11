#include "adt_7470_fan_controller.h"

#include <stddef.h>

#include "adt_7470_fan_controller_defs.h"

#define STM32_GPIO_STATE_SELECT_OUT_0 GPIO_STATE_LOW
#define STM32_GPIO_STATE_SELECT_OUT_1 GPIO_STATE_HIGH
// need to set interrupt once fan goes out of range
// PWM duty cycle is set from 0-100, in steps of 0.39 (0 - 0xFF)
StatusCode apt7470_set_speed(I2CPort port, uint8_t *speed, Fan FAN, uint16_t ADR7470_I2C_ADDRESS) {
  // need to select duty fan - correct fan duty cycle
  return i2c_write_reg(I2C_PORT_1, ADR7470_I2C_ADDRESS, ADT7470_PWM2_DUTY_CYCLE, speed,
                       SET_SPEED_NUM_BYTES);
}

static StatusCode prv_init_common(Adt7470Storage *storage) {
  // i2c pin stuff goes here
  I2CSettings i2c1_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = I2C1_SDA,
    .scl = I2C1_SCL,
  };

  i2c_init(I2C_PORT_1, &i2c1_settings);

  return STATUS_CODE_OK;
}

StatusCode bts_7240_init(Adt7470Storage *storage, Adt7470Settings *settings) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(Adt7470Storage));

  // 4 gpio's need to be set to PWM
  storage->fan_1_pin = settings->fan_1_pin;
  storage->fan_2_pin = settings->fan_2_pin;
  storage->fan_3_pin = settings->fan_3_pin;
  storage->fan_4_pin = settings->fan_4_pin;
  storage->interval_us = settings->interval_us;
  storage->callback = settings->callback;  // probably will use to check if tachometre reads 0
  storage->callback_context = settings->callback_context;

  // initialize the select pin
  GpioSettings select_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  // setting 4 fans to PWM output
  status_ok_or_return(gpio_init_pin(storage->fan_1_pin, &select_settings));
  status_ok_or_return(gpio_init_pin(storage->fan_2_pin, &select_settings));
  status_ok_or_return(gpio_init_pin(storage->fan_3_pin, &select_settings));
  status_ok_or_return(gpio_init_pin(storage->fan_4_pin, &select_settings));

  return prv_init_common(storage);
}
