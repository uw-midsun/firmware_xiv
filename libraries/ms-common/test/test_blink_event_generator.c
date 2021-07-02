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
  const uint32_t interval_us = 10000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    .default_state = BLINKER_STATE_ON,  // custom
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage;
  Event e;

  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));

  // make sure init doesn't start it
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_us(2 * interval_us);
  TEST_ASSERT_NOT_OK(event_process(&e));

  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));

  // make sure it raises an event immediately + respects the custom default state
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);  // opposite of default state

  // make sure it respects the interval
  delay_us(interval_us / 2);
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_us(interval_us / 2);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);  // make sure it actually toggles

  // make sure it only raises one event
  TEST_ASSERT_NOT_OK(event_process(&e));

  // make sure it raises more events
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
  TEST_ASSERT_OK(event_process(&e));  // don't raise immediately
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);  // resets to first_value
  delay_us(interval_us);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  // make sure we can restart without stopping first correctly
  blink_event_generator_start(&storage, TEST_EVENT_ID_2);  // different event_id to actually restart
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(0, e.data);  // resets to first_value
  delay_us(interval_us);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  blink_event_generator_stop(&storage);
}

// Test that restarting with the same event ID does nothing
void test_blink_event_generator_start_same_event_id(void) {
  const uint32_t interval_us = 10000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    .default_state = BLINKER_STATE_OFF,
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage;
  Event e;

  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));

  // Eat the first event
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_NOT_OK(event_process(&e));

  // Restart after 1/2 interval and make sure there's no initial event
  delay_us(interval_us / 2);
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_NOT_OK(event_process(&e));

  // Make sure the event is on time
  delay_us(interval_us / 2);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_NOT_OK(event_process(&e));

  blink_event_generator_stop(&storage);
}

// Test that blink_event_generator_init has the correct default default state of 0.
void test_blink_event_generator_init_default_first_value_is_0(void) {
  const uint32_t interval_us = 10000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    // default_state not specified
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage;

  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));

  Event e;
  TEST_ASSERT_OK(event_process(&e));  // get the event raised immediately
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);  // default state must default to 0, so transition to 1

  blink_event_generator_stop(&storage);
}

// Test that blink_event_generator_init errors with an invalid default state.
void test_blink_event_generator_init_invalid_first_value(void) {
  BlinkEventGeneratorSettings invalid_settings = {
    .interval_us = 1000,
    .default_state = NUM_BLINKER_STATES,
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage;
  TEST_ASSERT_NOT_OK(blink_event_generator_init(&storage, &invalid_settings));
}

// Test that we can manipulate multiple instances correctly.
// The times are real finicky in this test: interval_us1:interval_us2 = 5:8 works
void test_multiple_blink_event_generators(void) {
  Event e;

  // initialize the first
  const uint32_t interval_us1 = 11500;
  BlinkEventGeneratorSettings settings1 = {
    .interval_us = interval_us1,
    .default_state = BLINKER_STATE_ON,
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage1;
  TEST_ASSERT_OK(blink_event_generator_init(&storage1, &settings1));

  // initialize the second
  const uint32_t interval_us2 = 20000;  // must be greater than interval_us1
  BlinkEventGeneratorSettings settings2 = {
    .interval_us = interval_us2,
    .default_state = BLINKER_STATE_OFF,
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage2;
  TEST_ASSERT_OK(blink_event_generator_init(&storage2, &settings2));

  // start both at the same time, make sure events are raised immediately (and in order)
  TEST_ASSERT_OK(blink_event_generator_start(&storage1, TEST_EVENT_ID));
  TEST_ASSERT_OK(blink_event_generator_start(&storage2, TEST_EVENT_ID_2));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  // receive second event from the first one
  delay_us(interval_us1);  // time: interval_us1
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));  // only one event on the queue

  // receive second event from the second one
  delay_us(interval_us2 - interval_us1);  // time: interval_us2
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));

  // cancel the first one, make sure we still get events from the second one
  blink_event_generator_stop(&storage1);
  delay_us(interval_us1);                 // time: interval_us2 + interval_us1 < 2*interval_us2
  TEST_ASSERT_NOT_OK(event_process(&e));  // no event from the first one
  delay_us(interval_us2 - interval_us1);  // time: 2*interval_us2
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  // cancel the second one, restart the first one with a new event ID, make sure everything's valid
  blink_event_generator_stop(&storage2);
  TEST_ASSERT_OK(event_process(&e));  // eat the event from stop transitioning back to default
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_2, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  blink_event_generator_start(&storage1, TEST_EVENT_ID_3);
  TEST_ASSERT_OK(event_process(&e));  // event from the first one
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_3, e.id);
  TEST_ASSERT_EQUAL(0, e.data);  // defaults back to first_value
  delay_us(interval_us1);        // time: 2*interval_us2 + interval_u1
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID_3, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  delay_us(interval_us2 - interval_us1);  // time: 3*interval_us2
  TEST_ASSERT_NOT_OK(event_process(&e));  // second one is cancelled

  // cancel first one, make sure no more events are raised
  blink_event_generator_stop(&storage1);
  delay_us(2 * interval_us2);  // time: 5*interval_us2
  TEST_ASSERT_NOT_OK(event_process(&e));
}

// Test that blink_event_generator_stop and blink_event_generator_stop_silently return correctly.
void test_blink_event_generator_stop_return_value(void) {
  const uint32_t interval_us = 5000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage;
  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));

  // not started - false
  TEST_ASSERT_EQUAL(false, blink_event_generator_stop(&storage));
  TEST_ASSERT_EQUAL(false, blink_event_generator_stop_silently(&storage));

  // started - true
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_EQUAL(true, blink_event_generator_stop(&storage));
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_EQUAL(true, blink_event_generator_stop_silently(&storage));

  // already stopped - false
  TEST_ASSERT_EQUAL(false, blink_event_generator_stop(&storage));
  TEST_ASSERT_EQUAL(false, blink_event_generator_stop_silently(&storage));
}

// Test that if not in the default state, blink_event_generator_stop raises an event to transition
// back to the default state.
void test_blink_event_generator_stop_raises_last_event(void) {
  const uint32_t interval_us = 5000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    .default_state = BLINKER_STATE_ON,
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage;
  Event e;
  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));

  // start: transition to off
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));

  // stop: make sure we transition back to on
  TEST_ASSERT_EQUAL(true, blink_event_generator_stop(&storage));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));
}

// Test that whether in the default state or not, blink_event_generator_stop_silently does not
// raise any more events (yet still stops it).
void test_blink_event_generator_stop_silently_does_not_raise_last_event(void) {
  const uint32_t interval_us = 5000;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    .default_state = BLINKER_STATE_ON,
    .callback = NULL,
  };
  BlinkEventGeneratorStorage storage;
  Event e;
  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));

  // start: transition to off
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));

  // stop silently: make sure there's no transition
  TEST_ASSERT_EQUAL(true, blink_event_generator_stop_silently(&storage));
  TEST_ASSERT_NOT_OK(event_process(&e));

  // wait for a while: make sure it actually stopped
  delay_us(2 * interval_us);
  TEST_ASSERT_NOT_OK(event_process(&e));

  // start again: transition to off and then to on
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));
  delay_us(interval_us + 5);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);
  TEST_ASSERT_NOT_OK(event_process(&e));

  // stop silently: make sure there's no transition
  TEST_ASSERT_EQUAL(true, blink_event_generator_stop_silently(&storage));
  TEST_ASSERT_NOT_OK(event_process(&e));

  // wait and make sure it actually stopped again
  delay_us(2 * interval_us);
  TEST_ASSERT_NOT_OK(event_process(&e));
}

static uint16_t s_times_callback_called;
static BlinkerState s_passed_state;
static void *s_passed_context;

static void prv_counter_callback(BlinkerState new_state, void *context) {
  s_times_callback_called++;
  s_passed_context = context;
  s_passed_state = new_state;
}

// Test that the callback is called with the appropriate context.
void test_blink_event_generator_callback(void) {
  s_times_callback_called = 0;
  s_passed_context = NULL;
  s_passed_state = NUM_BLINKER_STATES;

  const uint32_t interval_us = 10000;
  void *arbitrary_context = &interval_us;
  BlinkEventGeneratorSettings settings = {
    .interval_us = interval_us,
    .default_state = BLINKER_STATE_ON,
    .callback = &prv_counter_callback,
    .callback_context = arbitrary_context,
  };
  BlinkEventGeneratorStorage storage;
  Event e = { 0 };
  TEST_ASSERT_OK(blink_event_generator_init(&storage, &settings));

  // initial event: make sure it was called
  TEST_ASSERT_OK(blink_event_generator_start(&storage, TEST_EVENT_ID));
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(0, e.data);
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(arbitrary_context, s_passed_context);
  TEST_ASSERT_EQUAL(BLINKER_STATE_OFF, s_passed_state);
  s_passed_context = NULL;
  s_passed_state = NUM_BLINKER_STATES;

  // make sure it's not called before the next event
  delay_us(interval_us / 2);
  TEST_ASSERT_EQUAL(1, s_times_callback_called);
  TEST_ASSERT_EQUAL(NULL, s_passed_context);
  TEST_ASSERT_EQUAL(NUM_BLINKER_STATES, s_passed_state);

  // next event: make sure it's called again
  delay_us(interval_us / 2 + 5);
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(TEST_EVENT_ID, e.id);
  TEST_ASSERT_EQUAL(1, e.data);
  TEST_ASSERT_EQUAL(2, s_times_callback_called);
  TEST_ASSERT_EQUAL(arbitrary_context, s_passed_context);
  TEST_ASSERT_EQUAL(BLINKER_STATE_ON, s_passed_state);

  TEST_ASSERT_EQUAL(true, blink_event_generator_stop(&storage));
}
