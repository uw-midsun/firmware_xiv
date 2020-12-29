#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "pcf8523_rtc.h"
#include "pcf8523_rtc_defs.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

I2CSettings i2c = {
  .speed = I2C_SPEED_FAST,         //
  .sda = TEST_CONFIG_PIN_I2C_SDA,  //
  .scl = TEST_CONFIG_PIN_I2C_SCL,  //
};

void setup_test(void) {
  gpio_init();
  i2c_init(I2C_PORT_1, &i2c);
}

void teardown_test(void) {}

void test_pcf8523_init_valid() {
  TEST_ASSERT_OK(pcf8523_init(I2C_PORT_1));
}

void test_get_time() {
  Pcf8523Time time = { 0 };
  TEST_ASSERT_OK(pcf8523_get_time(&time));
  // Make sure the value's have changed and are not all zero
  TEST_ASSERT_FALSE((time.seconds == 0) && (time.minutes == 0) && (time.hours == 0) &&
                    (time.days == 0) && (time.weekdays == 0) && (time.months == 0) &&
                    (time.years == 0));
}

void test_set_time() {
  Pcf8523Time time_set = {
    .seconds = 0,
    .minutes = 1,
    .hours = 0,
    .days = 1,
    .weekdays = 0,
    .months = 1,
    .years = 0,
  };

  Pcf8523Time time_get = { 0 };
  TEST_ASSERT_OK(pcf8523_set_time(&time_set));
  TEST_ASSERT_OK(pcf8523_get_time(&time_get));
  // Note: We don't check seconds since the timer will start as soon as we've finished setting it
  TEST_ASSERT_TRUE((time_get.minutes == 1) && (time_get.hours == 0) && (time_get.days == 1) &&
                   (time_get.weekdays == 0) && (time_get.months == 1) && (time_get.years == 0));
}

// Test setting time to 0
void test_invalid_set_time() {
  Pcf8523Time time_zero = { 0 };
  TEST_ASSERT_NOT_OK(pcf8523_set_time(&time_zero));

  Pcf8523Time time_overflow = {
    .seconds = 100,
    .minutes = 100,
    .hours = 100,
    .days = 100,
    .weekdays = 100,
    .months = 100,
    .years = 100,
  };
  TEST_ASSERT_NOT_OK(pcf8523_set_time(&time_overflow));
}
