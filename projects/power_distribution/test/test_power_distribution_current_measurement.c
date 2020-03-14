#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "power_distribution_current_measurement.h"
#include "power_distribution_current_measurement_config.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_I2C_PORT I2C_PORT_2

#define TEST_CONFIG_PIN_I2C_SCL \
  { GPIO_PORT_B, 10 }
#define TEST_CONFIG_PIN_I2C_SDA \
  { GPIO_PORT_B, 11 }

static int s_times_callback_called = 0;

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = TEST_CONFIG_PIN_I2C_SCL,
    .sda = TEST_CONFIG_PIN_I2C_SDA,
  };
  i2c_init(TEST_I2C_PORT, &i2c_settings);

  s_times_callback_called = 0;
}
void teardown_test(void) {}

static void prv_increment_callback(void *context) {
  s_times_callback_called++;
}

// Test that we can initialize, read a value, and stop with the front hardware configuration.
void test_power_distribution_current_measurement_front_hw_config_init_valid(void) {
  uint32_t interval_us = 2000;
  PowerDistributionCurrentSettings settings = {
    .interval_us = interval_us,
    .callback = &prv_increment_callback,
    .hw_config = FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG,
  };
  TEST_ASSERT_OK(power_distribution_current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // stop it and make sure the callback is no longer called
  TEST_ASSERT_OK(power_distribution_stop());
  delay_us(interval_us * 2);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Test that we can initialize, read a value, and stop with the rear hardware configuration.
void test_power_distribution_current_measurement_rear_hw_config_init_valid(void) {
  uint32_t interval_us = 2000;
  PowerDistributionCurrentSettings settings = {
    .interval_us = interval_us,
    .callback = &prv_increment_callback,
    .hw_config = REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG,
  };
  TEST_ASSERT_OK(power_distribution_current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // stop it and make sure the callback is no longer called
  TEST_ASSERT_OK(power_distribution_stop());
  delay_us(interval_us * 2);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Test that we can successfully get measurements with the front hardware config.
void test_power_distribution_current_measurement_front_hw_config_get_measurement_valid(void) {
  uint32_t interval_us = 2000;
  PowerDistributionCurrentSettings settings = {
    .interval_us = interval_us,
    .callback = &prv_increment_callback,
    .hw_config = FRONT_POWER_DISTRIBUTION_CURRENT_HW_CONFIG,
  };
  TEST_ASSERT_OK(power_distribution_current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // make sure we can get the storage
  PowerDistributionCurrentStorage *storage = power_distribution_current_measurement_get_storage();

  // print out the storage for debugging
  for (PowerDistributionCurrent i = 0; i < NUM_POWER_DISTRIBUTION_CURRENTS; i++) {
    LOG_DEBUG("front hw config: current %d is %d\r\n", i, storage->measurements[i]);
  }

  TEST_ASSERT_OK(power_distribution_stop());
}

// Test that we can successfully get measurements with the rear hardware config.
void test_power_distribution_current_measurement_rear_hw_config_get_measurement_valid(void) {
  uint32_t interval_us = 2000;
  PowerDistributionCurrentSettings settings = {
    .interval_us = interval_us,
    .callback = &prv_increment_callback,
    .hw_config = REAR_POWER_DISTRIBUTION_CURRENT_HW_CONFIG,
  };
  TEST_ASSERT_OK(power_distribution_current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // make sure we can get the storage
  PowerDistributionCurrentStorage *storage = power_distribution_current_measurement_get_storage();

  // print out the storage for debugging
  for (PowerDistributionCurrent i = 0; i < NUM_POWER_DISTRIBUTION_CURRENTS; i++) {
    LOG_DEBUG("rear hw config: current %d is %d\r\n", i, storage->measurements[i]);
  }

  TEST_ASSERT_OK(power_distribution_stop());
}

// Test that init errors with invalid hardware config.
void test_power_distribution_current_measurement_invalid_hw_config(void) {
  const I2CAddress test_i2c_address = 0x74;  // resolve valid
  PowerDistributionCurrentHardwareConfig hw_config = {
    .i2c_port = TEST_I2C_PORT,
    .dsel_i2c_addresses = (I2CAddress[]){ test_i2c_address },
    .num_dsel_i2c_addresses = 1,
    .bts7200s = (PowerDistributionBts7200Data[]){ {
        .dsel_pin = { .i2c_address = test_i2c_address, .pin = PCA9539R_PIN_IO0_0 },
        .current_0 = 0,
        .current_1 = 1,
        .mux_selection = 0,
    } },
    .num_bts7200_channels = 1,
    .bts7040s = (PowerDistributionBts7040Data[]){ {
        .current = 2,
        .mux_selection = 1,
    } },
    .num_bts7040_channels = 1,
    .mux_address =
        {
            .bit_width = 4,
            .sel_pins =
                {
                    { .port = GPIO_PORT_A, .pin = 0 },  //
                    { .port = GPIO_PORT_A, .pin = 1 },  //
                    { .port = GPIO_PORT_A, .pin = 2 },  //
                    { .port = GPIO_PORT_A, .pin = 3 },  //
                },
            .mux_output_pin = { .port = GPIO_PORT_A, .pin = 0 },  //
            .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 1 },  //
        },
  };
  PowerDistributionCurrentSettings settings = {
    .interval_us = 2000,
    .hw_config = hw_config,
  };

  // invalid number of BTS7200 channels
  settings.hw_config.num_bts7200_channels = MAX_POWER_DISTRIBUTION_BTS7200_CHANNELS + 1;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.num_bts7200_channels = 1;

  // invalid number of BTS7040 channels
  settings.hw_config.num_bts7040_channels = MAX_POWER_DISTRIBUTION_BTS7040_CHANNELS + 1;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.num_bts7040_channels = 1;

  // invalid BTS7200 DSEL address
  settings.hw_config.bts7200s[0].dsel_pin.pin = NUM_PCA9539R_GPIO_PINS;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.bts7200s[0].dsel_pin.pin = PCA9539R_PIN_IO0_0;

  // invalid BTS7200 currents
  settings.hw_config.bts7200s[0].current_0 = NUM_POWER_DISTRIBUTION_CURRENTS;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.bts7200s[0].current_0 = 0;
  settings.hw_config.bts7200s[0].current_1 = NUM_POWER_DISTRIBUTION_CURRENTS;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.bts7200s[0].current_1 = 1;

  // invalid BTS7040 current
  settings.hw_config.bts7040s[0].current = NUM_POWER_DISTRIBUTION_CURRENTS;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.bts7040s[0].current = 2;

  // invalid mux sel address
  settings.hw_config.mux_address.sel_pins[0].port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.mux_address.sel_pins[0].port = GPIO_PORT_A;

  // invalid mux output pin
  settings.hw_config.mux_address.mux_output_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.mux_address.mux_output_pin.port = GPIO_PORT_A;

  // invalid mux enable pin
  settings.hw_config.mux_address.mux_enable_pin.port = NUM_GPIO_PORTS;
  TEST_ASSERT_NOT_OK(power_distribution_current_measurement_init(&settings));
  settings.hw_config.mux_address.mux_enable_pin.port = GPIO_PORT_A;

  // otherwise valid
  TEST_ASSERT_OK(power_distribution_current_measurement_init(&settings));
  TEST_ASSERT_OK(power_distribution_stop());
}
