// PCF8523 is a real-time clock (RTC) IC
// 1. Define the bit definitions
// 2. Need to send data with I2C to the slave, note that the registers
// auto increment after each read/write,
// For write mode: data transfer is terminated by writing STOP or START
// for next data transfer. Note the slave must ACK each time.
// For Read mode: send the address first with I2C with the register
// to read from
// An ACK is generated after each byte read
// Don't ACK to stop the reading
// To acknowledge, the recieving device must pull SDA low
// 3. Need to actually write the data to send and figure out which registers
// to write to
#pragma once

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
