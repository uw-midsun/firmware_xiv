#include "pcf8523_rtc.h"

#include "bcd.h"
#include "gpio.h"
#include "i2c.h"
#include "i2c_mcu.h"
#include "log.h"
#include "pcf8523_rtc_defs.h"
#include "status.h"

#define STOP_CR1_SETTINGS (1 << PCF8523_CR1_STOP)
// Countdown and watchdog timers disabled
#define DEFAULT_CR2_SETTINGS 0

static I2CPort s_port;
static Pcf8523CrystalLoadCapacitance s_cap;

StatusCode pcf8523_init(I2CPort i2c_port, Pcf8523Settings *settings) {
  s_port = i2c_port;
  s_cap = settings->cap;
  // Write the address of the first control register and then
  // send the data for the 3 control registers
  // Note that the addr of the control register auto-increments
  // After each byte is sent
  uint8_t data[NUM_CONTROL_REG + 1];
  data[0] = CR1;
  // Set load capacitance
  data[1] = s_cap << PCF8523_CR1_CAP_SEL;
  data[2] = DEFAULT_CR2_SETTINGS;
  // Set power management setting
  // Note battery low detection function is disabled
  data[3] = settings->power << PCF8523_CR3_PM;

  return i2c_write(s_port, PCF8523_I2C_ADDR, data, (sizeof(data)));
}

StatusCode pcf8523_get_time(Pcf8523Time *time) {
  // Set starting register address (this will auto-increment)
  uint8_t starting_reg = SECONDS;
  i2c_write(s_port, PCF8523_I2C_ADDR, &starting_reg, 1);

  // Read time registers
  uint8_t data[NUM_TIME_REG];
  i2c_read(s_port, PCF8523_I2C_ADDR, data, (sizeof(data)));

  // If the 7th bit is enabled if the oscillator has stopped
  if ((time->seconds) & (1 << 7)) {
    LOG_WARN("Clock integrity is not guaranteed; oscillator has stopped or been interrupted");
    // Disable 7th bit to ensure we get proper values when converting to decimal
    time->seconds &= ~(1 << 7);
  }
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
  // Stop the timer
  uint8_t stop[2] = { CR1, STOP_CR1_SETTINGS };
  i2c_write(s_port, PCF8523_I2C_ADDR, stop, (sizeof(stop)));

  // Write time to registers
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
  i2c_write(s_port, PCF8523_I2C_ADDR, data, (sizeof(data)));

  // Restart the timer
  uint8_t restart[2] = { CR1, s_cap << PCF8523_CR1_CAP_SEL };
  i2c_write(s_port, PCF8523_I2C_ADDR, restart, (sizeof(restart)));

  return STATUS_CODE_OK;
}
