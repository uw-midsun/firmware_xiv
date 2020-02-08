#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "rear_power_distribution_current_measurement.h"
#include "rear_power_distribution_current_measurement_config.h"
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
    .hw_config = REAR_POWER_DISTRIBUTION_HW_CONFIG,  // generic HW config
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
    .hw_config = REAR_POWER_DISTRIBUTION_HW_CONFIG,
  };
  TEST_ASSERT_OK(rear_power_distribution_current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // make sure we can get the storage
  RearPowerDistributionCurrentStorage *storage =
      rear_power_distribution_current_measurement_get_storage();

  // print out the storage for debugging
  for (uint8_t i = 0; i < REAR_POWER_DISTRIBUTION_HW_CONFIG.num_bts7200_channels; i++) {
    LOG_DEBUG("measurement %d is %d\r\n", i, storage->measurements[i]);
  }

  TEST_ASSERT_OK(rear_power_distribution_stop());
}

// Test that init errors with invalid hardware config.
void test_rear_power_distribution_current_measurement_invalid_hw_config(void) {
  RearPowerDistributionCurrentHardwareConfig hw_config = {
    .num_bts7200_channels = 4,
    .dsel_i2c_address = 1,
    .bts7200_to_dsel_address =
        (Mcp23008GpioAddress[]){
            { .i2c_address = 1, .pin = 0 },
            { .i2c_address = 1, .pin = 1 },
            { .i2c_address = 1, .pin = 2 },
            { .i2c_address = 1, .pin = 3 },
        },
    .mux_address =
        {
            .sel_pins =
                {
                    { .port = GPIO_PORT_A, .pin = 0 },  //
                    { .port = GPIO_PORT_A, .pin = 1 },  //
                    { .port = GPIO_PORT_A, .pin = 2 },  //
                },
            .mux_output_pin = { .port = GPIO_PORT_A, .pin = 0 },  //
        },
    .bts7200_to_mux_select = (uint8_t[]){ 0, 1, 2, 3 }
  };
  RearPowerDistributionCurrentSettings settings = {
    .interval_us = 2000,
    .hw_config = hw_config,
  };

  // invalid DSEL pin address
  settings.hw_config.bts7200_to_dsel_address[0].pin = NUM_MCP23008_GPIO_PINS;
  TEST_ASSERT_NOT_OK(rear_power_distribution_current_measurement_init(&settings));
  settings.hw_config.bts7200_to_dsel_address[0].pin = 0;

  // invalid mux sel address
  settings.hw_config.mux_address.sel_pins[0].port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(rear_power_distribution_current_measurement_init(&settings));
  settings.hw_config.mux_address.sel_pins[0].port = GPIO_PORT_A;

  // invalid mux output pin
  settings.hw_config.mux_address.mux_output_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(rear_power_distribution_current_measurement_init(&settings));
  settings.hw_config.mux_address.mux_output_pin.port = GPIO_PORT_A;

  // otherwise valid
  TEST_ASSERT_OK(rear_power_distribution_current_measurement_init(&settings));
}
