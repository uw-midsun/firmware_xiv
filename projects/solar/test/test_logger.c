#include "logger.h"

#include "data_store.h"
#include "event_queue.h"
#include "log.h"
#include "solar_boards.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

static Event s_data_ready_event = { .id = DATA_READY_EVENT };

void setup_test(void) {
  event_queue_init();
  data_store_init();

  logger_init(SOLAR_BOARD_6_MPPTS);
}
void teardown_test(void) {}

// Logger's job is to send human-readable output to the console, which makes it difficult to
// automatically test it without tightly coupling the tests to the output format, which makes
// changes hard. (Plus we'd have to inject a logging callback into the module or mock LOG_DEBUG or
// printf, and that's messy.)
// Instead, most of these tests will never fail - a human has to look at the output and determine
// if it's right.

// Test that we can initialize with a valid number of MPPTs.
void test_logger_valid_initialization(void) {
  TEST_ASSERT_OK(logger_init(SOLAR_BOARD_5_MPPTS));
  TEST_ASSERT_OK(logger_init(SOLAR_BOARD_6_MPPTS));
}

// Test that initialization fails with an invalid number of MPPTs.
void test_logger_invalid_initialization(void) {
  TEST_ASSERT_NOT_OK(logger_init(MAX_SOLAR_BOARD_MPPTS + 1));
}

// Test |logger_process_event| returns true when it's passed a data ready event and false otherwise.
void test_logger_process_event_return_value(void) {
  LOG_DEBUG("Logging expected:\n");
  TEST_ASSERT_TRUE(logger_process_event(&s_data_ready_event));

  LOG_DEBUG("No logging expected:\n");
  Event non_data_ready_event = { .id = DATA_READY_EVENT + 1 };
  TEST_ASSERT_FALSE(logger_process_event(&non_data_ready_event));
}

// Test that initializing with fewer MPPTs makes logger output fewer data points.
// A human-only test.
void test_logger_fewer_mppts(void) {
  logger_init(SOLAR_BOARD_5_MPPTS);
  LOG_DEBUG("Logging from only 5 MPPTs expected:\n");
  logger_process_event(&s_data_ready_event);
}

// Test that everything is output as unset if we don't set any data points.
// A human-only test.
void test_logger_nothing_set(void) {
  LOG_DEBUG("All data points unset expected:\n");
  logger_process_event(&s_data_ready_event);
}

// Test that everything is output with units if we set all the data points.
// A human-only test.
void test_logger_everything_set(void) {
  const uint32_t set_value = 20;
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    data_store_set(data_point, set_value);
  }

  LOG_DEBUG("All data points expected with numeric value %lu:\n",
            (long unsigned int)set_value);  // NOLINT(runtime/int)
  logger_process_event(&s_data_ready_event);
}

// Test that the signed data points (currently just CURRENT) are output correctly.
// A human-only test.
void test_logger_signed_values(void) {
  const int32_t signed_value = -20;
  const uint32_t unsigned_value = (uint32_t)signed_value;
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    data_store_set(data_point, unsigned_value);
  }

  LOG_DEBUG("Signed data points expected with %ld, unsigned data points with %lu:\n",
            (long int)signed_value, (long unsigned int)unsigned_value);  // NOLINT(runtime/int)
  logger_process_event(&s_data_ready_event);
}
