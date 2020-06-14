#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "sense.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_SENSE_PERIOD_US 50000  // 50ms, should be okay

static SenseSettings test_settings = {
  .sense_period_us = TEST_SENSE_PERIOD_US,
};

static uint8_t s_times_data_store_done_called = 0;

static uint8_t s_times_callback_called = 0;
static void *s_callback_context = NULL;

static uint8_t s_times_callback_2_called = 0;
static void *s_callback_2_context = NULL;

// We mock |data_store_done| to make sure it's being called without relying on data_store behaviour.
StatusCode TEST_MOCK(data_store_done)(void) {
  s_times_data_store_done_called++;
  return STATUS_CODE_OK;
}

static void prv_callback(void *context) {
  s_times_callback_called++;
  s_callback_context = context;
}

static void prv_callback_2(void *context) {
  s_times_callback_2_called++;
  s_callback_2_context = context;
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  s_times_data_store_done_called = 0;
  s_times_callback_called = 0;
  s_callback_context = NULL;
  s_times_callback_2_called = 0;
  s_callback_2_context = NULL;
}
void teardown_test(void) {}

// Perform a happy-path test of the sense cycle with a single callback. Also tests multiple cycles.
void test_sense_cycle_single_callback_multiple_cycles(void) {
  TEST_ASSERT_OK(sense_init(&test_settings));

  uint8_t context;  // throwaway variable to serve as context
  TEST_ASSERT_OK(sense_register(prv_callback, &context));
  TEST_ASSERT_EQUAL(0, s_times_callback_called);  // callback should not be called yet
  TEST_ASSERT_EQUAL(0, s_times_data_store_done_called);

  // start the cycle, callback and data_store_done should be called immediately
  sense_start();
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(1, s_times_data_store_done_called);
  TEST_ASSERT_EQUAL(&context, s_callback_context);  // context was passed
  s_callback_context = NULL;

  // make sure the next round isn't too early
  delay_us(TEST_SENSE_PERIOD_US / 2);
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(1, s_times_data_store_done_called);

  // there should be another cycle after cumulative TEST_SENSE_PERIOD_US, add 1ms to be sure
  delay_us(TEST_SENSE_PERIOD_US / 2 + 1000);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  TEST_ASSERT_EQUAL(2, s_times_data_store_done_called);
  TEST_ASSERT_EQUAL(&context, s_callback_context);

  // one more round
  delay_us(TEST_SENSE_PERIOD_US);
  TEST_ASSERT_EQUAL(3, s_times_callback_called);
  TEST_ASSERT_EQUAL(3, s_times_data_store_done_called);

  // stop it and make sure it's really stopped
  TEST_ASSERT_EQUAL(true, sense_stop());
  delay_us(2 * TEST_SENSE_PERIOD_US);
  TEST_ASSERT_EQUAL(3, s_times_callback_called);
  TEST_ASSERT_EQUAL(3, s_times_data_store_done_called);

  // no events should have been raised since we mocked data_store_done
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that we can have two callbacks with distinct contexts.
void test_sense_cycle_two_callbacks_with_contexts(void) {
  TEST_ASSERT_OK(sense_init(&test_settings));

  uint8_t context, context2;  // throwaway context variables
  TEST_ASSERT_OK(sense_register(prv_callback, &context));
  TEST_ASSERT_OK(sense_register(prv_callback_2, &context2));

  // nothing has been called yet
  TEST_ASSERT_EQUAL(0, s_times_callback_called);
  TEST_ASSERT_EQUAL(0, s_times_callback_2_called);
  TEST_ASSERT_EQUAL(0, s_times_data_store_done_called);

  // make sure all callbacks are called with correct contexts
  sense_start();
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(1, s_times_callback_2_called);
  TEST_ASSERT_EQUAL(1, s_times_data_store_done_called);
  TEST_ASSERT_EQUAL(&context, s_callback_context);
  TEST_ASSERT_EQUAL(&context2, s_callback_2_context);

  // stop, make sure it's really stopped
  TEST_ASSERT_EQUAL(true, sense_stop());
  delay_us(2 * TEST_SENSE_PERIOD_US);
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(1, s_times_callback_2_called);
  TEST_ASSERT_EQUAL(1, s_times_data_store_done_called);

  // no events since we mocked data_store_done
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that we can have several callbacks.
void test_sense_cycle_many_callbacks(void) {
  const size_t num_callbacks = MAX_SENSE_CALLBACKS;
  TEST_ASSERT_OK(sense_init(&test_settings));

  for (size_t i = 0; i < num_callbacks; i++) {
    TEST_ASSERT_OK(sense_register(prv_callback, NULL));
  }

  // all of the callbacks are called
  sense_start();
  TEST_ASSERT_EQUAL(num_callbacks, s_times_callback_called);
  TEST_ASSERT_EQUAL(1, s_times_data_store_done_called);
  sense_stop();
}

// Test that trying to register too many callbacks gives STATUS_CODE_RESOURCE_EXHAUSTED, but this
// doesn't break the existing callbacks.
void test_registering_too_many_callbacks(void) {
  TEST_ASSERT_OK(sense_init(&test_settings));

  for (size_t i = 0; i < MAX_SENSE_CALLBACKS; i++) {
    TEST_ASSERT_OK(sense_register(prv_callback, NULL));
  }

  // too many callbacks gives STATUS_CODE_RESOURCE_EXHAUSTED
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, sense_register(prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, sense_register(prv_callback, NULL));
  TEST_ASSERT_EQUAL(STATUS_CODE_RESOURCE_EXHAUSTED, sense_register(prv_callback, NULL));

  // it only calls MAX_SENSE_CALLBACKS callbacks
  sense_start();
  TEST_ASSERT_EQUAL(MAX_SENSE_CALLBACKS, s_times_callback_called);
  TEST_ASSERT_EQUAL(1, s_times_data_store_done_called);
  sense_stop();
}

// Test that passing NULL into functions gives the appropriate status codes.
void test_passing_null(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_init(NULL));
  sense_init(&test_settings);  // reset the callbacks for |sense_register|
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, sense_register(NULL, NULL));
}

// Test that |sense_stop| returns true iff it stops an ongoing sense cycle.
void test_sense_stop_return_value(void) {
  TEST_ASSERT_OK(sense_init(&test_settings));
  TEST_ASSERT_EQUAL(false, sense_stop());
  sense_start();
  TEST_ASSERT_EQUAL(true, sense_stop());
  TEST_ASSERT_EQUAL(false, sense_stop());
}
