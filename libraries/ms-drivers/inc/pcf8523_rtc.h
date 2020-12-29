#pragma once

#include <stdint.h>
#include "i2c.h"
#include "i2c_mcu.h"
#include "status.h"

#define I2C_ADDR 0x68

typedef struct {
  I2CPort i2c_port;
  I2CSettings *i2c_settings;
} Pcf8523Settings;

typedef struct {
  uint8_t seconds;   // 0 to 59
  uint8_t minutes;   // 0 to 59
  uint8_t hours;     // 0 to 23 (in 24 hr mode)
  uint8_t days;      // 1 to 31
  uint8_t weekdays;  // 0 to 6
  uint8_t months;    // 1 to 12
  uint8_t years;     // 0 to 99
} Pcf8523Time;

StatusCode pcf8523_init(Pcf8523Settings *settings);

StatusCode pcf8523_get_time(Pcf8523Time *time);

StatusCode pcf8523_set_time(Pcf8523Time *time);
