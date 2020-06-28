#include "adt_7470_fan_controller.h"

#include <stddef.h>

#include "adt_7470_fan_controller_defs.h"

// need to set interrupt once fan goes out of range
// PWM duty cycle is set from 0-100, in steps of 0.39 (0 - 0xFF)
StatusCode adt7470_set_speed(I2CPort port, uint8_t speed, uint8_t FAN_PWM_ADDR,
                             uint16_t ADT7476A_I2C_ADDRESS) {
  // need to select duty fan - correct fan duty cycle
  return i2c_write_reg(port, ADT7476A_I2C_ADDRESS, ADT7470_PWM2_DUTY_CYCLE, speed,
                       SET_SPEED_NUM_BYTES);
}

// static StatusCode prv_init_common(Adt7470Storage *storage) {
//   // i2c pin stuff goes here
//   I2CSettings i2c1_settings = {
//     .speed = I2C_SPEED_FAST,
//     .sda = I2C1_SDA,
//     .scl = I2C1_SCL,
//   };

//   i2c_init(I2C_PORT_1, &i2c1_settings);

//   return STATUS_CODE_OK;
// }

StatusCode adt7470_init(Adt7470Storage *storage, Adt7470Settings *settings) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  memset(storage, 0, sizeof(Adt7470Storage));

  // 4 gpio's need to be set to PWM
  storage->fan_1_pin = settings->fan_1_pin;
  storage->fan_2_pin = settings->fan_2_pin;
  storage->fan_3_pin = settings->fan_3_pin;
  storage->fan_4_pin = settings->fan_4_pin;
  storage->interval_ms = settings->interval_ms;
  storage->i2c = settings->i2c;
  storage->i2c_addr = settings->i2c_addr;
  storage->i2c_settings = settings->i2c_settings;
  storage->callback = settings->callback;  // probably will use to check if tachometre reads 0
  storage->callback_context = settings->callback_context;

  // // initialize the select pin
  // GpioSettings select_settings = {
  //   .direction = GPIO_DIR_OUT,
  //   .state = GPIO_STATE_LOW,
  //   .resistor = GPIO_RES_NONE,
  //   .alt_function = GPIO_ALTFN_NONE,
  // };

  // // setting 4 fans to PWM output
  // status_ok_or_return(gpio_init_pin(storage->fan_1_pin, &select_settings));
  // status_ok_or_return(gpio_init_pin(storage->fan_2_pin, &select_settings));
  // status_ok_or_return(gpio_init_pin(storage->fan_3_pin, &select_settings));
  // status_ok_or_return(gpio_init_pin(storage->fan_4_pin, &select_settings));

  status_ok_or_return(i2c_init(storage->i2c, &storage->i2c_settings));

  // init pwm

  StatusCode status;

  // uint8_t read_byte[NUM_BYTES_TO_READ] = { 0 };
  // uint8_t write_byte[NUM_BYTES_TO_WRITE] = { 0 };

  uint8_t read_byte;
  uint8_t write_byte;

  status = i2c_read_reg(storage->i2c, storage->i2c_addr, ADT7476A_FAN_MODE_REGISTER_1, read_byte,
                        NUM_BYTES_TO_READ);

  write_byte |= ADT7470_MANUAL_MODE_MASK;

  return STATUS_CODE_OK;
}
