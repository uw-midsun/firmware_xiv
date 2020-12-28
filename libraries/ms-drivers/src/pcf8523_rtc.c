#include "pcf8523_rtc.h"
#include <time.h>
#include "gpio.h"
#include "i2c.h"
#include "pcf8523_rtc_defs.h"
#include "status.h"

static I2CPort port;

StatusCode pcf8523_init(Pcf8523Settings *settings) {
  gpio_init();
  port = settings->i2c_port;
  i2c_init(port, settings->i2c_settings);
}

StatusCode pcf8523_get_time(tm *time) {}

StatusCode pcf8523_set_time(tm *time) {}

StatusCode pcf8523_reset() {
  uint8_t data[2];
  data[0] = CR1;
  data[1] = CR1_SOFTWARE_RESET;
  i2c_write(port, I2C_ADDR, data, (sizeof(data)));
  return STATUS_CODE_OK;
}
