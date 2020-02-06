#include "blink_event_generator.h"
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {
  // process any unprocessed events
  Event e;
  while (status_ok(event_process(&e))) {
  }
}

#define TEST_EVENT_ID 10
#define TEST_EVENT_ID_2 11
#define TEST_EVENT_ID_3 12

// Test that we can initialize, receive events at the proper times, stop, restart, and stop.
void test_blink_event_generator_valid(void) {
  const uint32_t interval_us = 5000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    .first_value = 1,  // custom
  };
  BlinkEventGeneratorStorage storage;
  Event e;

  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));

  // make sure init doesn't start it
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_us(2 * interval_us);
  TEST_ASSERT_NOT_OK(event_process(&e));

  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));

  // make sure it doesn't raise an event immediately
  TEST_ASSERT_NOT_OK(event_process(&e));

  // make sure it respects the interval
  delay_us(interval_us / 2);
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_us(interval_us / 2);
  TEST_ASSERT_OK(event_process(&e));

  // make sure it respects the custom first value and the event ID is valid
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  // make sure it only raises one event
  TEST_ASSERT_NOT_OK(event_process(&e));

  // make sure it raises more events and actually toggles
  delay_us(interval_us);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  delay_us(interval_us);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  // make sure stop works
  blink_event_generator_stop(&storage);
  delay_us(2 * interval_us);
  TEST_ASSERT_NOT_OK(event_process(&e));  // no event there

  // make sure we can restart correctly
  blink_event_generator_start(&storage, TEST_EVENT_ID);
  TEST_ASSERT_NOT_OK(event_process(&e));  // don't raise immediately
  delay_us(interval_us);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);  // resets to first_value

  // make sure we can restart without stopping first correctly
  blink_event_generator_start(&storage, TEST_EVENT_ID_2);
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_us(interval_us);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(1, e.data);  // resets to first_value
}

// Test that blink_event_generator_init has the correct default first value of 0.
void test_blink_event_generator_init_default_first_value_is_0(void) {
  const uint32_t interval_us = 5000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    // first_value not specified
  };
  BlinkEventGeneratorStorage storage;

  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  delay_us(interval_us);

  Event e;
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);  // first value must default to 0

  blink_event_generator_stop(&storage);
}

// Test that blink_event_generator_init errors with an invalid first value.
void test_blink_event_generator_init_invalid_first_value(void) {
  BlinkEventGeneratorSettings invalid_settings = {
    .interval_us = 1000,
    .first_value = 0xBEEF,
  };
  BlinkEventGeneratorStorage storage;
  TEST_ASSERT_NOT_OK(blink_event_generator_init(&storage, &invalid_settings));
}

// Test that we can manipulate multiple instances correctly.
void test_multiple_blink_event_generators(void) {
  Event e;

  // initialize the first
  const uint32_t interval_us1 = 5000;
  BlinkEventGeneratorSettings settings1 = {
    .interval_us = interval_us1,
    .first_value = 0,
  };
  BlinkEventGeneratorStorage storage1;
  TEST_ASSERT_OK(blink_event_generator_init(&storage1, &settings1));

  // initialize the second
  const uint32_t interval_us2 = 7000;  // must be greater than interval_us1
  BlinkEventGeneratorSettings settings2 = {
    .interval_us = interval_us2,
    .first_value = 1,
  };
  BlinkEventGeneratorStorage storage2;
  TEST_ASSERT_OK(blink_event_generator_init(&storage2, &settings2));

  // start both at the same time, make sure no event is raised immediately
  TEST_ASSERT_OK(blink_event_generator_start(&storage1, TEST_EVENT_ID));
  TEST_ASSERT_OK(blink_event_generator_start(&storage2, TEST_EVENT_ID_2));
  TEST_ASSERT_NOT_OK(event_process(&e));

  // receive first event from the first one
  delay_us(interval_us1);  // time: interval_us1
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));  // only one event on the queue

  // receive first event from the second one
  delay_us(interval_us2 - interval_us1);  // time: interval_us2
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(1, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));

  // cancel the first one, make sure we still get events from the second one
  blink_event_generator_stop(&storage1);
  delay_us(interval_us1);                 // time: interval_us2 + interval_us1 < 2*interval_us2
  TEST_ASSERT_NOT_OK(event_process(&e));  // no event from the first one
  delay_us(interval_us2 - interval_us1);  // time: 2*interval_us2
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(0, e.data);

  // cancel the second one, restart the first one with a new event ID, make sure everything's valid
  blink_event_generator_stop(&storage2);
  blink_event_generator_start(&storage1, TEST_EVENT_ID_3);
  delay_us(interval_us1);             // time: 2*interval_us2 + interval_u1
  TEST_ASSERT_OK(event_process(&e));  // event from the first one
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_3, e.id);
  TEST_ASSERT_EQUAL(0, e.data);  // defaults back to first_value

  delay_us(interval_us2 - interval_us1);  // time: 3*interval_us2
  TEST_ASSERT_NOT_OK(event_process(&e));  // second one is cancelled

  // cancel first one, make sure no more events are raised
  blink_event_generator_stop(&storage1);
  delay_us(2 * interval_us2);  // time: 5*interval_us2
  TEST_ASSERT_NOT_OK(event_process(&e));
}

// Test that blink_event_generator_stop's return value is correct.
void test_blink_event_generator_stop_return_value(void) {
  const uint32_t interval_us = 5000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
  };
  BlinkEventGeneratorStorage storage;
  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));

  // not started - false
  TEST_ASSERT_EQUAL(false, blink_event_generator_stop(&storage));

  // started - true
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_EQUAL(true, blink_event_generator_stop(&storage));

  // already stopped - false
  TEST_ASSERT_EQUAL(false, blink_event_generator_stop(&storage));
}
