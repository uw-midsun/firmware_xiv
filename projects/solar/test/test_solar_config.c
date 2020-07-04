#include "log.h"
#include "sense_mcp3427.h"
#include "solar_config.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}
void teardown_test(void) {}

// Test that we can initialize the config returned by |config_get_sense_mcp3427_settings|
void test_initializing_sense_mcp3427_config(void) {
  SenseMcp3427Settings settings;
  TEST_ASSERT_OK(config_get_sense_mcp3427_settings(SOLAR_BOARD_5_MPPTS, &settings));
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
  TEST_ASSERT_OK(config_get_sense_mcp3427_settings(SOLAR_BOARD_6_MPPTS, &settings));
  TEST_ASSERT_OK(sense_mcp3427_init(&settings));
}

// Test that passing invalid arguments fails gracefully.
void test_invalid_args(void) {
  SenseMcp3427Settings settings;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    config_get_sense_mcp3427_settings(MAX_SOLAR_BOARD_MPPTS + 1, &settings));
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS,
                    config_get_sense_mcp3427_settings(SOLAR_BOARD_5_MPPTS, NULL));
}
