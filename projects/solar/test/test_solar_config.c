#include "solar_config.h"

#include "adc.h"
#include "data_store.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "sense.h"
#include "sense_mcp3427.h"
#include "sense_mppt.h"
#include "sense_temperature.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  event_queue_init();
  data_store_init();
  adc_init(ADC_MODE_SINGLE);

  SenseSettings sense_settings = { .sense_period_us = 100000 };
  sense_init(&sense_settings);
}
void teardown_test(void) {}

// Test that we can initialize the configs returned by |config_get_sense_temperature_settings|.
// This is separated into two tests to avoid exhausting the sense callbacks.
void test_initializing_sense_temperature_config_5_mppts(void) {
  SenseTemperatureSettings settings;
  TEST_ASSERT_OK(config_get_sense_temperature_settings(SOLAR_BOARD_5_MPPTS, &settings));
  TEST_ASSERT_OK(sense_temperature_init(&settings));
}
void test_initializing_sense_temperature_config_6_mppts(void) {
  SenseTemperatureSettings settings;
  TEST_ASSERT_OK(config_get_sense_temperature_settings(SOLAR_BOARD_6_MPPTS, &settings));
  TEST_ASSERT_OK(sense_temperature_init(&settings));
}

// Test that we can initialize the config returned by |config_get_sense_mcp3427_settings|.
// Again separate to avoid exhausting callbacks.
void test_initializing_sense_mcp3427_config_5_mppts(void) {
  SenseMcp3427Settings settings;
  TEST_ASSERT_OK(config_get_sense_mcp3427_settings(SOLAR_BOARD_5_MPPTS, &settings));
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
}
void test_initializing_sense_mcp3427_config_6_mppts(void) {
  SenseMcp3427Settings settings;
  TEST_ASSERT_OK(config_get_sense_mcp3427_settings(SOLAR_BOARD_6_MPPTS, &settings));
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
}

// Test that we can initialize the config returned by |config_get_sense_mppt_settings|.
// Again separate to avoid exhausting callbacks.
void test_initializing_sense_mppt_config_5_mppts(void) {
  SenseMpptSettings settings;
  TEST_ASSERT_OK(config_get_sense_mppt_settings(SOLAR_BOARD_5_MPPTS, &settings));
  TEST_ASSERT_OK(sense_mppt_init(&settings));
}
void test_initializing_sense_mppt_config_6_mppts(void) {
  SenseMpptSettings settings;
  TEST_ASSERT_OK(config_get_sense_mppt_settings(SOLAR_BOARD_6_MPPTS, &settings));
  TEST_ASSERT_OK(sense_mppt_init(&settings));
}

// Test that passing invalid arguments fails gracefully.
void test_invalid_args(void) {
  StatusCode status;

  SenseMcp3427Settings mcp3427_settings;
  status = config_get_sense_mcp3427_settings(MAX_SOLAR_BOARD_MPPTS + 1, &mcp3427_settings);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
  status = config_get_sense_mcp3427_settings(SOLAR_BOARD_5_MPPTS, NULL);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);

  SenseTemperatureSettings temp_settings;
  status = config_get_sense_temperature_settings(MAX_SOLAR_BOARD_MPPTS + 1, &temp_settings);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
  status = config_get_sense_temperature_settings(SOLAR_BOARD_5_MPPTS, NULL);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);

  SenseMpptSettings mppt_settings;
  status = config_get_sense_mppt_settings(MAX_SOLAR_BOARD_MPPTS + 1, &mppt_settings);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
  status = config_get_sense_mppt_settings(SOLAR_BOARD_5_MPPTS, NULL);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}
