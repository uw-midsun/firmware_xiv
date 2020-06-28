#pragma once

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "i2c.h"
#include "interrupt.h"
#include "soft_timer.h"

typedef void (*Adt7476ADataCallback)(void *context);

typedef struct {
  GpioAddress *smbalert_pin;
  uint32_t interval_ms;
  Adt7476ADataCallback callback;  // set to NULL for no callback
  void *callback_context;
  I2CPort i2c;
  I2CAddress i2c_read_addr;
  I2CAddress i2c_write_addr;
  I2CSettings i2c_settings;
} Adt7476aSettings;

typedef struct {
  GpioAddress *smbalert_pin;
  uint32_t interval_ms;
  SoftTimerId timer_id;
  Adt7476ADataCallback callback;
  void *callback_context;
  I2CPort i2c;
  I2CAddress i2c_read_addr;
  I2CAddress i2c_write_addr;
  I2CSettings i2c_settings;
} Adt7476aStorage;

// probably a better name for these
typedef enum { FAN_1 = 0, FAN_2, FAN_3, FAN_4 } FAN_NUM;

// Initialize the Adt7476a with the given settings; the select pin is an STM32 GPIO pin.
StatusCode adt7476a_init(Adt7476aStorage *storage, Adt7476aSettings *settings);

// Translate and write the new speed
StatusCode adt7476a_set_speed(I2CPort port, uint8_t speed, uint8_t FAN_PWM_ADDR,
                              uint8_t ADR7470_I2C_ADDRESS);
