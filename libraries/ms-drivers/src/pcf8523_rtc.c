#include "pcf8523_rtc.h"
#include "bcd.h"
#include "gpio.h"
#include "i2c.h"
#include "pcf8523_rtc_defs.h"
#include "status.h"

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
  data[1] = 1 << TIME_12_24 | 1 << CAP_SEL;
  // Default settings for CR2 and CR3
  data[2] = 0;
  data[3] = 0;

  i2c_write(port, I2C_ADDR, data, (sizeof(data)));
}

StatusCode pcf8523_get_time(Pcf8523Time *time) {
  // Set starting register address (this will auto-increment)
  i2c_write(port, I2C_ADDR, SECONDS, 1);

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

StatusCode pcf8523_set_time(Pcf8523Time *time) {}
