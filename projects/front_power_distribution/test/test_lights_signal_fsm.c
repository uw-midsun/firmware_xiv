// Note: we rely upon the fact that blink_event_generator raises an event immediately to detect
// state transitions.

#include "delay.h"
#include "event_queue.h"
#include "front_power_distribution_events.h"
#include "interrupt.h"
#include "lights_signal_fsm.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_BLINK_INTERVAL_US 6000

#define TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(event_id, event_data) \
  do {                                                               \
    const Event e = { 0 };                                           \
    TEST_ASSERT_OK(event_process(&e));                               \
    TEST_ASSERT_EQUAL((event_id), e.id);                             \
    TEST_ASSERT_EQUAL((event_data), e.data);                         \
  } while (0)

#define TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED() \
  do {                                              \
    const Event e = { 0 };                          \
    StatusCode s = event_process(&e);               \
    TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, s);        \
  } while (0)

typedef enum {
  TEST_SIGNAL_EVENT_LEFT = 0,
  TEST_SIGNAL_EVENT_RIGHT,
  TEST_SIGNAL_EVENT_HAZARD,
  NUM_TEST_SIGNAL_EVENTS,
} TestSignalEvents;

static SignalFsmStorage s_storage;
static const SignalFsmSettings s_settings = {
  .signal_left_event = TEST_SIGNAL_EVENT_LEFT,
  .signal_right_event = TEST_SIGNAL_EVENT_RIGHT,
  .signal_hazard_event = TEST_SIGNAL_EVENT_HAZARD,
  .blink_interval_us = TEST_BLINK_INTERVAL_US,
};

static const Event s_left_on_event = { .id = TEST_SIGNAL_EVENT_LEFT, .data = 1 };
static const Event s_left_off_event = { .id = TEST_SIGNAL_EVENT_LEFT, .data = 0 };
static const Event s_right_on_event = { .id = TEST_SIGNAL_EVENT_RIGHT, .data = 1 };
static const Event s_right_off_event = { .id = TEST_SIGNAL_EVENT_RIGHT, .data = 0 };
static const Event s_hazard_on_event = { .id = TEST_SIGNAL_EVENT_HAZARD, .data = 1 };
static const Event s_hazard_off_event = { .id = TEST_SIGNAL_EVENT_HAZARD, .data = 0 };

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();
}
void teardown_test(void) {}

// Test that we can successfully initialize.
void test_lights_signal_fsm_init_ok(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));
}

// Test that we actually blink.
void test_lights_signal_fsm_blinking(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // transition to hazard
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_on_event));

  // make sure we're blinking
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 1);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // transition to none and make sure we're no longer blinking (but that we turn off the light)
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 0);
  delay_us(2 * TEST_BLINK_INTERVAL_US);
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();
}

// Test transitioning: none -> left -> none -> right -> none.
// Events given: left on, left off, right on, right off.
void test_lights_signal_fsm_none_left_none_right_none(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // begin: none
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left on: transition to left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT, 1);

  // left off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT, 0);

  // right on: transition to right
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_right_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT, 1);

  // right off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_right_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT, 0);
}

// Test transitioning: none -> hazard -> hazard-left -> left -> none
// Events given: hazard on, left on, hazard off, left off
void test_lights_signal_fsm_none_hazard_hazardleft_left_none(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // begin: none
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard on: transition to hazard
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 1);

  // left on: transition to hazard-left (no behaviour change)
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard off: transition to left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT, 1);

  // left off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT, 0);
}

// Test transitioning: none -> left -> hazard-left -> hazard -> none
// Events given: left on, hazard on, left off, hazard off
void test_lights_signal_fsm_none_left_hazardleft_hazard_none(void) {
  TEST_ASSERT_OK(lights_signal_fsm_init(&s_storage, &s_settings));

  // begin: none
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // left on: transition to left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT, 1);

  // hazard on: transition to hazard-left
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_on_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT, 0);
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 1);

  // left off: transition to hazard (no behaviour change)
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_left_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_NO_EVENT_RAISED();

  // hazard off: transition to none
  TEST_ASSERT_OK(lights_signal_fsm_process_event(&s_storage, &s_hazard_off_event));
  TEST_LIGHTS_SIGNAL_ASSERT_EVENT_RAISED(FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, 0);
}
