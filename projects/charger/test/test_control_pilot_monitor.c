#include "gpio.h"
#include "event_queue.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "pwm_input.h"

#include "charger_control_pilot_monitor.h"
#include "charger_events.h"

void setup_test(void) {
    event_queue_init();
    gpio_init();
    gpio_it_init();
    TEST_ASSERT_OK(control_pilot_monitor_init());
}

void teardown_test(void) {}

void prv_event_handle(Event e) {
    TEST_ASSERT_EQUAL(e.id, PWM_READING_VALUE);
}

void test_control_pilot_event_handle() {
    event_raise(PWM_READING_REQUEST, 0);
}