#include "current_measurement.h"

#include <stdint.h>

#include "adc.h"
#include "current_measurement_config.h"
#include "delay.h"
#include "gpio.h"
#include "i2c.h"
#include "interrupt.h"
#include "log.h"
#include "output.h"
#include "output_config.h"
#include "pca9539r_gpio_expander.h"
#include "pin_defs.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

// Note: since output handles the intricacies of reading from BTS7200 and GPIO doesn't support
// current sense, we only test reading from BTS7040 outputs.

#define TEST_OUTPUT_0 0
#define TEST_OUTPUT_1 1

#define TEST_NO_CURRENT_SENSE_OUTPUT 2  // a GPIO output which doesn't support reading current

#define TEST_I2C_ADDRESS 0x74
#define TEST_ENABLE_PIN_0 \
  { TEST_I2C_ADDRESS, PCA9539R_PIN_IO0_0 }
#define TEST_ENABLE_PIN_1 \
  { TEST_I2C_ADDRESS, PCA9539R_PIN_IO0_1 }
#define TEST_MUX_SEL_0 0
#define TEST_MUX_SEL_1 1
#define TEST_GPIO_ADDRESS \
  { GPIO_PORT_A, 0 }

static size_t s_times_callback_called = 0;

static OutputConfig s_test_output_config = {
  .specs =
      {
          [TEST_OUTPUT_0] =
              {
                  .type = OUTPUT_TYPE_BTS7040,
                  .on_front = true,
                  .bts7040_spec =
                      {
                          .enable_pin = TEST_ENABLE_PIN_0,
                          .mux_selection = TEST_MUX_SEL_0,
                      },
              },
          [TEST_OUTPUT_1] =
              {
                  .type = OUTPUT_TYPE_BTS7040,
                  .on_front = true,
                  .bts7040_spec =
                      {
                          .enable_pin = TEST_ENABLE_PIN_1,
                          .mux_selection = TEST_MUX_SEL_1,
                      },
              },
          [TEST_NO_CURRENT_SENSE_OUTPUT] =
              {
                  .type = OUTPUT_TYPE_GPIO,
                  .on_front = true,
                  .gpio_spec =
                      {
                          .address = TEST_GPIO_ADDRESS,
                      },
              },
      },
  .mux_address =
      {
          .bit_width = 4,
          .sel_pins =
              {
                  PD_MUX_SEL1_PIN,
                  PD_MUX_SEL2_PIN,
                  PD_MUX_SEL3_PIN,
                  PD_MUX_SEL4_PIN,
              },
      },
  .mux_output_pin = PD_MUX_OUTPUT_PIN,
  .mux_enable_pin = PD_MUX_ENABLE_PIN,
  .i2c_addresses = (I2CAddress[]){ TEST_I2C_ADDRESS },
  .num_i2c_addresses = 1,
  .i2c_port = PD_I2C_PORT,
};

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  adc_init(ADC_MODE_SINGLE);

  I2CSettings i2c_settings = {
    .speed = I2C_SPEED_FAST,
    .scl = PD_I2C_SCL_PIN,
    .sda = PD_I2C_SDA_PIN,
  };
  i2c_init(PD_I2C_PORT, &i2c_settings);

  output_init(&s_test_output_config, true);

  s_times_callback_called = 0;
}
void teardown_test(void) {}

static void prv_increment_callback(void *context) {
  s_times_callback_called++;
}

// Test that we can initialize, read a value, and stop with the front hardware configuration.
void test_power_distribution_current_measurement_front_hw_config_init_valid(void) {
  output_init(&g_combined_output_config, true);

  uint32_t interval_us = 2000;
  CurrentMeasurementSettings settings = {
    .interval_us = interval_us,
    .callback = prv_increment_callback,
    .hw_config = &g_front_current_measurement_config,
  };
  TEST_ASSERT_OK(current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // stop it and make sure the callback is no longer called
  TEST_ASSERT_OK(current_measurement_stop());
  delay_us(interval_us * 2);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Test that we can initialize, read a value, and stop with the rear hardware configuration.
void test_power_distribution_current_measurement_rear_hw_config_init_valid(void) {
  output_init(&g_combined_output_config, false);

  uint32_t interval_us = 2000;
  CurrentMeasurementSettings settings = {
    .interval_us = interval_us,
    .callback = prv_increment_callback,
    .hw_config = &g_rear_current_measurement_config,
  };
  TEST_ASSERT_OK(current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // stop it and make sure the callback is no longer called
  TEST_ASSERT_OK(current_measurement_stop());
  delay_us(interval_us * 2);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}

// Test that we can successfully get measurements with the front hardware config.
void test_power_distribution_current_measurement_front_hw_config_get_measurement_valid(void) {
  output_init(&g_combined_output_config, true);

  uint32_t interval_us = 2000;
  CurrentMeasurementSettings settings = {
    .interval_us = interval_us,
    .callback = prv_increment_callback,
    .hw_config = &g_front_current_measurement_config,
  };
  TEST_ASSERT_OK(current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // make sure we can get the storage
  CurrentMeasurementStorage *storage = current_measurement_get_storage();
  TEST_ASSERT_NOT_NULL(storage);

  // print out the storage for debugging
  for (Output i = 0; i < NUM_OUTPUTS; i++) {
    LOG_DEBUG("front hw config: output %d's current is %d mA\n", i, storage->measurements[i]);
  }

  TEST_ASSERT_OK(current_measurement_stop());
}

// Test that we can successfully get measurements with the rear hardware config.
void test_power_distribution_current_measurement_rear_hw_config_get_measurement_valid(void) {
  output_init(&g_combined_output_config, false);

  uint32_t interval_us = 2000;
  CurrentMeasurementSettings settings = {
    .interval_us = interval_us,
    .callback = prv_increment_callback,
    .hw_config = &g_rear_current_measurement_config,
  };
  TEST_ASSERT_OK(current_measurement_init(&settings));

  // init should read values immediately
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // wait for the callback to be called
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // make sure we can get the storage
  CurrentMeasurementStorage *storage = current_measurement_get_storage();
  TEST_ASSERT_NOT_NULL(storage);

  // print out the storage for debugging
  for (Output i = 0; i < NUM_OUTPUTS; i++) {
    LOG_DEBUG("rear hw config: output %d's current is %d mA\n", i, storage->measurements[i]);
  }

  TEST_ASSERT_OK(current_measurement_stop());
}

// Test that init errors with invalid hardware config.
void test_power_distribution_current_measurement_invalid_hw_config(void) {
  CurrentMeasurementConfig hw_config = {
    .outputs_to_read =
        (Output[]){
            TEST_OUTPUT_0,
            TEST_OUTPUT_1,
        },
    .num_outputs_to_read = 2,
  };
  CurrentMeasurementSettings settings = {
    .interval_us = 2000,
    .hw_config = &hw_config,
  };

  // invalid number of outputs
  hw_config.num_outputs_to_read = NUM_OUTPUTS + 1;
  TEST_ASSERT_NOT_OK(current_measurement_init(&settings));
  hw_config.num_outputs_to_read = 2;

  // otherwise valid
  TEST_ASSERT_OK(current_measurement_init(&settings));
  TEST_ASSERT_OK(current_measurement_stop());
}

// Test that we don't stop, but a warning is issued, when an output doesn't support current sense.
// We can't actually test for the presence of the warnings, but look for them :)
void test_warns_with_no_current_output(void) {
  uint32_t interval_us = 2000;
  CurrentMeasurementConfig hw_config = {
    .outputs_to_read =
        (Output[]){
            TEST_NO_CURRENT_SENSE_OUTPUT,
        },
    .num_outputs_to_read = 1,
  };
  CurrentMeasurementSettings settings = {
    .interval_us = interval_us,
    .hw_config = &hw_config,
    .callback = prv_increment_callback,
  };

  // read occurs immediately
  LOG_WARN("Warning about no current sense on output %d expected:\n", TEST_NO_CURRENT_SENSE_OUTPUT);
  TEST_ASSERT_OK(current_measurement_init(&settings));
  TEST_ASSERT_EQUAL(1, s_times_callback_called);

  // and we can read again even after a no-current-sense output
  LOG_WARN("Warning about no current sense on output %d expected:\n", TEST_NO_CURRENT_SENSE_OUTPUT);
  delay_us(interval_us);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);

  // we stop correctly even with a no-current-sense output
  TEST_ASSERT_OK(current_measurement_stop());
  delay_us(interval_us * 2);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
}
