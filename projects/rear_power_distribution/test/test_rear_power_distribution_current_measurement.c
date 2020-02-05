#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "rear_power_distribution_current_measurement.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static int s_times_callback_called = 0;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .sda = REAR_POWER_DISTRIBUTION_CURRENT_I2C_SDA_ADDRESS,
    .scl = REAR_POWER_DISTRIBUTION_CURRENT_I2C_SCL_ADDRESS,
  };
  i2c_init(I2C_PORT_2, &i2c_settings);

  s_times_callback_called = 0;
}
void teardown_test(void) {}

static void prv_increment_callback(void *context) {
  s_times_callback_called++;
}

// Test that we can initialize, read a value, and stop.
void test_rear_power_distribution_current_measurement_init_valid(void) {
  uint32_t interval_us = 2000;
  RearPowerDistributionCurrentSettings settings = {
    .interval_us = interval_us,
    .callback = &prv_increment_callback,
  };
  TEST_ASSERT_OK(rear_power_distribution_current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // stop it and make sure the callback is no longer called
  TEST_ASSERT_OK(rear_power_distribution_stop());
  delay_us(interval_us * 2);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Test that we can successfully get measurements.
void test_rear_power_distribution_current_measurement_get_measurement_valid(void) {
  uint32_t interval_us = 2000;
  RearPowerDistributionCurrentSettings settings = {
    .interval_us = interval_us,
    .callback = &prv_increment_callback,
  };
  TEST_ASSERT_OK(rear_power_distribution_current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // make sure we can get the storage
  RearPowerDistributionCurrentStorage storage;
  TEST_ASSERT_OK(rear_power_distribution_current_measurement_get_storage(&storage));

  // print out the storage for debugging
  for (RearPowerDistributionCurrentMeasurement i = 0; i < NUM_REAR_POWER_DISTRIBUTION_CURRENTS;
       i++) {
    LOG_DEBUG("measurement %d is %d\r\n", i, storage.measurements[i]);
  }

  TEST_ASSERT_OK(rear_power_distribution_stop());
}
