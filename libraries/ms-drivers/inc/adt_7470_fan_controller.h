#pragma once

#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "soft_timer.h"

typedef void (*Adt7470DataCallback)();

typedef struct {
  I2CPort i2c_port;
  GpioAddress *fan_1_pin;
  GpioAddress *fan_2_pin;
  GpioAddress *fan_3_pin;
  GpioAddress *fan_4_pin;
  uint32_t interval_us;
  Adt7470DataCallback callback;  // set to NULL for no callback
  void *callback_context;
} Adt7470Settings;

typedef struct {
  GpioAddress *fan_1_pin;
  GpioAddress *fan_2_pin;
  GpioAddress *fan_3_pin;
  GpioAddress *fan_4_pin;
  uint32_t interval_us;
  SoftTimerId timer_id;
  Adt7470DataCallback callback;
  void *callback_context;
} Adt7470Storage;

// probably a better name for these
typedef enum { FAN_1 = 0, FAN_2, FAN_3, FAN_4 } Fan;

// Initialize the Adt7200 with the given settings; the select pin is an STM32 GPIO pin.
StatusCode adt_7200_init_stm32(Adt7470Storage *storage, Adt7470Settings *settings);

// Translate and write the new speed
StatusCode apt7470_set_speed(I2CPort port, uint8_t *status, Fan Fan, uint16_t ADR7470_I2C_ADDRESS);
