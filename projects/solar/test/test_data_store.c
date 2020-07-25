#include "data_store.h"
#include "event_queue.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

#define VALID_TEST_DATA_POINT DATA_POINT_CURRENT
#define VALID_TEST_VALUE 0x1337

void setup_test(void) {
  event_queue_init();
}
void teardown_test(void) {}

// Test that we can set and get a value from the data store.
void test_data_store_set_and_get_basic(void) {
  TEST_ASSERT_OK(data_store_init());
  uint32_t value;
  TEST_ASSERT_OK(data_store_set(VALID_TEST_DATA_POINT, VALID_TEST_VALUE));
  TEST_ASSERT_OK(data_store_get(VALID_TEST_DATA_POINT, &value));
  TEST_ASSERT_EQUAL(VALID_TEST_VALUE, value);
}

// Test that we can test whether a value is set correctly.
void test_data_store_get_is_set_basic(void) {
  TEST_ASSERT_OK(data_store_init());
  bool is_set;
  TEST_ASSERT_OK(data_store_get_is_set(VALID_TEST_DATA_POINT, &is_set));
  TEST_ASSERT_EQUAL(false, is_set);
  TEST_ASSERT_OK(data_store_set(VALID_TEST_DATA_POINT, VALID_TEST_VALUE));
  TEST_ASSERT_OK(data_store_get_is_set(VALID_TEST_DATA_POINT, &is_set));
  TEST_ASSERT_EQUAL(true, is_set);
}

// Test that every data point works.
void test_data_store_set_and_get_and_get_is_set_thorough(void) {
  TEST_ASSERT_OK(data_store_init());
  uint32_t value;
  bool is_set;
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    TEST_ASSERT_OK(data_store_get_is_set(data_point, &is_set));
    TEST_ASSERT_EQUAL(false, is_set);
    TEST_ASSERT_OK(data_store_set(data_point, VALID_TEST_VALUE));
    TEST_ASSERT_OK(data_store_get_is_set(data_point, &is_set));
    TEST_ASSERT_EQUAL(true, is_set);
    TEST_ASSERT_OK(data_store_get(data_point, &value));
    TEST_ASSERT_EQUAL(VALID_TEST_VALUE, value);
  }
}

// Test that |data_store_set| responds correctly on an invalid index.
void test_data_store_set_invalid_index(void) {
  TEST_ASSERT_OK(data_store_init());
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_set(NUM_DATA_POINTS, 0));
}

// Test that |data_store_get| responds correctly on an invalid index.
void test_data_store_get_invalid_index(void) {
  TEST_ASSERT_OK(data_store_init());
  uint32_t value;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_get(NUM_DATA_POINTS, &value));
}

// Test that |data_store_get| responds correctly on a null pointer.
void test_data_store_get_null_pointer(void) {
  TEST_ASSERT_OK(data_store_init());
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_get(VALID_TEST_DATA_POINT, NULL));
}

// Test that |data_store_get_is_set| responds correctly on an invalid index.
void test_data_store_get_is_set_invalid_index(void) {
  TEST_ASSERT_OK(data_store_init());
  bool is_set;
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_get_is_set(NUM_DATA_POINTS, &is_set));
}

// Test that |data_store_get_is_set| responds correctly on a null pointer.
void test_data_store_get_is_set_null_pointer(void) {
  TEST_ASSERT_OK(data_store_init());
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, data_store_get_is_set(VALID_TEST_DATA_POINT, NULL));
}

// Test that |data_store_done| correctly raises DATA_READY_EVENTs.
void test_data_store_done_raises_events(void) {
  TEST_ASSERT_OK(data_store_init());
  Event e;
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(data_store_done());
  MS_TEST_HELPER_ASSERT_EVENT(e, DATA_READY_EVENT, 0);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
