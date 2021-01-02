#pragma once

// The pcf8523 is a real time clock that
// allows the user to read the time and rewrite the time
// Requires GPIO and I2C to be initialized

// There are features such as timers, alarms, battery low detection, clock pulses
// that have not been implemented but are present on the PCF8523.
// This clock is also in 24hr mode by default

#include <stdint.h>
#include "i2c.h"
#include "i2c_mcu.h"
#include "status.h"

#define PCF8523_I2C_ADDR 0x68

typedef enum {
  SEVEN_PF,
  TWELVE_POINT_FIVE_PF,
} Pcf8523CrystalLoadCapacitance;

typedef struct {
  uint8_t seconds;   // 0 to 59
  uint8_t minutes;   // 0 to 59
  uint8_t hours;     // 0 to 23 (in 24 hr mode)
  uint8_t days;      // 1 to 31
  uint8_t weekdays;  // 0 to 6
  uint8_t months;    // 1 to 12
  uint8_t years;     // 0 to 99
} Pcf8523Time;

StatusCode pcf8523_init(I2CPort i2c_port, Pcf8523CrystalLoadCapacitance cap);

StatusCode pcf8523_get_time(Pcf8523Time *time);

StatusCode pcf8523_set_time(Pcf8523Time *time);
