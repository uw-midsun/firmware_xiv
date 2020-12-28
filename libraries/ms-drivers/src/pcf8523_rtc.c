#include "pcf8523_rtc.h"
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

StatusCode pcf8523_get_time(Pcf8523Time *time) {}

StatusCode pcf8523_set_time(Pcf8523Time *time) {}
