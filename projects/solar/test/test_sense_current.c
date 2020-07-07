#include "data_store.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "mcp3427_adc.h"
#include "ms_test_helpers.h"
#include "sense.h"
#include "sense_current.h"
#include "soft_timer.h"
#include "solar_config.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SENSED_CH1_VALUE 0x1337
#define TEST_SENSED_CH2_VALUE 0xDEAD
#define TEST_STORED_VALUE TEST_SENSED_CH1_VALUE  // only CH1 is used

// these is updated when |sense_register| is called
static SenseCallback s_sense_callback;

StatusCode TEST_MOCK(sense_register)(SenseCallback callback, void *context) {
  s_sense_callback = callback;
  return STATUS_CODE_OK;
}

// these are updated when the mcp3427 mocked functions are called
static Mcp3427Callback s_mcp3427_callback;
static Mcp3427FaultCallback s_mcp3427_fault_callback;
static uint8_t s_times_mcp3427_start_called;

StatusCode TEST_MOCK(mcp3427_register_callback)(Mcp3427Storage *storage, Mcp3427Callback callback,
                                                void *context) {
  s_mcp3427_callback = callback;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(mcp3427_register_fault_callback)(Mcp3427Storage *storage,
                                                      Mcp3427FaultCallback callback,
                                                      void *context) {
  s_mcp3427_fault_callback = callback;
  return STATUS_CODE_OK;
}

// we mock this mostly to prevent actual MCP3427 cycles from starting up
StatusCode TEST_MOCK(mcp3427_start)(void) {
  s_times_mcp3427_start_called++;
  return STATUS_CODE_OK;
}

// these are updated when |data_store_set| is called
static uint8_t s_times_data_store_set_called;
static DataPoint s_data_store_set_data_point;
static uint16_t s_data_store_set_value;

// set this to set the return code of |data_store_set|
static StatusCode s_data_store_set_return_code;

StatusCode TEST_MOCK(data_store_set)(DataPoint point, uint16_t value) {
  s_times_data_store_set_called++;
  s_data_store_set_data_point = point;
  s_data_store_set_value = value;
  return s_data_store_set_return_code;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  // The dependencies on I2C and sense are mocked out, so we don't initialize them

  s_sense_callback = NULL;
  s_mcp3427_callback = NULL;
  s_mcp3427_fault_callback = NULL;
  s_times_mcp3427_start_called = 0;
  s_times_data_store_set_called = 0;
  s_data_store_set_data_point = NUM_DATA_POINTS;  // invalid
  s_data_store_set_value = 0;
  s_data_store_set_return_code = STATUS_CODE_OK;
}
void teardown_test(void) {}

// Test that a sense current cycle works.
void test_sense_current_normal_cycle(void) {
  TEST_ASSERT_OK(sense_current_init(&sense_current_settings));

  // make sure the various registration functions were called
  TEST_ASSERT_NOT_NULL(s_sense_callback);
  TEST_ASSERT_NOT_NULL(s_mcp3427_callback);
  TEST_ASSERT_NOT_NULL(s_mcp3427_fault_callback);
  TEST_ASSERT_EQUAL(1, s_times_mcp3427_start_called);   // |mcp3427_start| was called
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);  // |data_store_set| not called yet

  // call the MCP3427 callback, make sure nothing external happened
  s_mcp3427_callback(TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE, NULL);
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);

  // call the sense cycle callback, make sure |data_store_set| is called
  s_sense_callback(NULL);
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);
  TEST_ASSERT_EQUAL(DATA_POINT_CURRENT, s_data_store_set_data_point);
  TEST_ASSERT_EQUAL(TEST_STORED_VALUE, s_data_store_set_value);

  // reset the data point and value for a second cycle
  s_data_store_set_data_point = NUM_DATA_POINTS;
  s_data_store_set_value = 0;

  // one more cycle, same thing
  s_mcp3427_callback(TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE, NULL);
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);
  s_sense_callback(NULL);
  TEST_ASSERT_EQUAL(2, s_times_data_store_set_called);
  TEST_ASSERT_EQUAL(DATA_POINT_CURRENT, s_data_store_set_data_point);
  TEST_ASSERT_EQUAL(TEST_STORED_VALUE, s_data_store_set_value);

  // no fault events were raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that |data_store_set| isn't called when the sense cycle is before the MCP3427 callback.
void test_sense_current_data_not_ready(void) {
  TEST_ASSERT_OK(sense_current_init(&sense_current_settings));

  // just to make sure we avoid segfaults, make sure register callbacks were called
  TEST_ASSERT_NOT_NULL(s_sense_callback);
  TEST_ASSERT_NOT_NULL(s_mcp3427_callback);

  s_sense_callback(NULL);
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);  // |data_store_set| wasn't called
  s_sense_callback(NULL);
  TEST_ASSERT_EQUAL(0, s_times_data_store_set_called);  // it still wasn't called

  s_mcp3427_callback(TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE, NULL);
  s_sense_callback(NULL);
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);  // now it was called after mcp3427 callback
}

// Test that we log a warning when |data_store_set| returns not ok.
// We can't automatically test that a log occurs, so you have to manually verify that it occurs.
void test_sense_current_data_store_set_not_ok(void) {
  TEST_ASSERT_OK(sense_current_init(&sense_current_settings));
  TEST_ASSERT_NOT_NULL(s_sense_callback);
  TEST_ASSERT_NOT_NULL(s_mcp3427_callback);

  s_data_store_set_return_code = STATUS_CODE_INTERNAL_ERROR;
  LOG_WARN("Testing bad status code from data_store_set, there should be a warning here:\n");
  s_mcp3427_callback(TEST_SENSED_CH1_VALUE, TEST_SENSED_CH2_VALUE, NULL);
  s_sense_callback(NULL);  // warning here
  TEST_ASSERT_EQUAL(1, s_times_data_store_set_called);
}

// Test that initializing with NULL settings fails gracefully.
void test_sense_current_init_null(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_current_init(NULL));
}
