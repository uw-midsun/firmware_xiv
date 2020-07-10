#include "solar_config.h"

#include "log.h"
#include "sense_temperature.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}
void teardown_test(void) {}

// Test that we can initialize the config returned by |config_get_sense_temperature_settings|
void test_initializing_sense_temperature_config(void) {
  SenseTemperatureSettings settings;
  TEST_ASSERT_OK(config_get_sense_temperature_settings(SOLAR_BOARD_5_MPPTS, &settings));
  TEST_ASSERT_OK(sense_temperature_init(&settings));
  TEST_ASSERT_OK(config_get_sense_temperature_settings(SOLAR_BOARD_6_MPPTS, &settings));
  TEST_ASSERT_OK(sense_temperature_init(&settings));
}

// Test that passing invalid arguments fails gracefully.
void test_invalid_args(void) {
  SenseTemperatureSettings temp_settings;
  StatusCode status =
      config_get_sense_temperature_settings(MAX_SOLAR_BOARD_MPPTS + 1, &temp_settings);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
  status = config_get_sense_temperature_settings(SOLAR_BOARD_5_MPPTS, NULL);
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, status);
}
