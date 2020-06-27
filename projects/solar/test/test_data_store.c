#include "data_store.h"
#include "event_queue.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  event_queue_init();
}
void teardown_test(void) {}

// Test that we can set and get a value from the data store.
void test_data_store_set_and_get_basic(void) {
  uint16_t value;
  TEST_ASSERT_OK(data_store_set(DATA_POINT_VOLTAGE_1, 0x1337));
  TEST_ASSERT_OK(data_store_get(DATA_POINT_VOLTAGE_1, &value));
  TEST_ASSERT_EQUAL(0x1337, value);
}

// Test that every data point works.
void test_data_store_set_and_get_thorough(void) {
  uint16_t value;
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    TEST_ASSERT_OK(data_store_set(data_point, 0xDEAD));
    TEST_ASSERT_OK(data_store_get(data_point, &value));
    TEST_ASSERT_EQUAL(0xDEAD, value);
  }
}

// Test that |data_store_set| responds correctly on an invalid index.
void test_data_store_set_invalid_index(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_set(NUM_DATA_POINTS, 0));
}

// Test that |data_store_get| responds correctly on an invalid index.
void test_data_store_get_invalid_index(void) {
  uint16_t value;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_get(NUM_DATA_POINTS, &value));
}

// Test that |data_store_get| responds correctly on a null pointer.
void test_data_store_get_null_pointer(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_get(DATA_POINT_VOLTAGE_1, NULL));
}

// Test that |data_store_done| correctly raises DATA_READY_EVENTs.
void test_data_store_done_raises_events(void) {
  Event e;
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(data_store_done());
  MS_TEST_HELPER_ASSERT_EVENT(e, DATA_READY_EVENT, 0);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
