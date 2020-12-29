#include "pcf8523_rtc.h"
#include "bcd.h"
#include "gpio.h"
#include "i2c.h"
#include "i2c_mcu.h"
#include "pcf8523_rtc_defs.h"
#include "status.h"

#define DEFAULT_CR1_SETTINGS (1 << TIME_12_24 | 1 << CAP_SEL)
#define STOP_CR1_SETTINGS (1 << TIME_12_24 | 1 << STOP | 1 << CAP_SEL)

static I2CPort port;

StatusCode pcf8523_init(Pcf8523Settings *settings) {
  gpio_init();
  port = settings->i2c_port;
  i2c_init(port, settings->i2c_settings);

  // Write the address of the first control register and then
  // send the data for the 3 control registers
  // Note that the addr of the control register auto-increments
  // After each byte is sent
  uint8_t data[NUM_CONTROL_REG + 1];
  data[0] = CR1;
  // Set 24 hr time and 12.5pF load capacitance
  data[1] = DEFAULT_CR1_SETTINGS;
  // Default settings for CR2 and CR3
  data[2] = 0;
  data[3] = 0;

  i2c_write(port, I2C_ADDR, data, (sizeof(data)));
  return STATUS_CODE_OK;
}

StatusCode pcf8523_get_time(Pcf8523Time *time) {
  // Set starting register address (this will auto-increment)
  uint8_t starting_reg = SECONDS;
  i2c_write(port, I2C_ADDR, &starting_reg, 1);

  // Read time registers
  uint8_t data[NUM_TIME_REG];
  i2c_read(port, I2C_ADDR, data, (sizeof(data)));

  // Store time
  time->seconds = bcd_to_dec(data[0]);
  time->minutes = bcd_to_dec(data[1]);
  time->hours = bcd_to_dec(data[2]);
  time->days = bcd_to_dec(data[3]);
  time->weekdays = bcd_to_dec(data[4]);
  time->months = bcd_to_dec(data[5]);
  time->years = bcd_to_dec(data[6]);
  return STATUS_CODE_OK;
}

StatusCode pcf8523_set_time(Pcf8523Time *time) {
  if (time->seconds > 59 || time->minutes > 59 || time->hours > 23 || time->days > 31 ||
      time->days < 1 || time->weekdays > 6 || time->months > 12 || time->months < 1 ||
      time->years > 99) {
    return STATUS_CODE_INVALID_ARGS;
  }
  uint8_t data[NUM_TIME_REG + 1];
  data[0] = SECONDS;
  // Convert data to bcd
  data[1] = dec_to_bcd(time->seconds);
  data[2] = dec_to_bcd(time->minutes);
  data[3] = dec_to_bcd(time->hours);
  data[4] = dec_to_bcd(time->days);
  data[5] = dec_to_bcd(time->weekdays);
  data[6] = dec_to_bcd(time->months);
  data[7] = dec_to_bcd(time->years);

  // Stop the timer
  uint8_t stop[2] = { CR1, STOP_CR1_SETTINGS };
  i2c_write(port, I2C_ADDR, stop, (sizeof(stop)));

  // Write time to registers
  i2c_write(port, I2C_ADDR, data, (sizeof(data)));

  // Restart the timer
  uint8_t restart[2] = { CR1, DEFAULT_CR1_SETTINGS };
  i2c_write(port, I2C_ADDR, restart, (sizeof(restart)));

  return STATUS_CODE_OK;
}
