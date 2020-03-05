#include "event_queue.h"
#include "gpio.h"
#include "ms_test_helpers.h"
#include "pwm_input.h"
#include "test_helpers.h"

#include "charger_control_pilot_monitor.h"
#include "charger_events.h"

void setup_test(void) {
  event_queue_init();
  gpio_init();
  // Tests gpio init
  TEST_ASSERT_OK(control_pilot_monitor_init());
}

void teardown_test(void) {}

void prv_event_handle(Event e) {
  TEST_ASSERT_EQUAL(e.id, PWM_READING_VALUE);
}

// Tests that raising a reading request will cause an event to be raised
// with the read value
void test_control_pilot_event_handle() {
  event_raise(PWM_READING_REQUEST, 0);
  Event e = { 0 };
  // Process event raised here
  event_process(&e);
  handle_pwm_event(e);

  // Process event raised by module
  event_process(&e);
  prv_event_handle(e);
}
