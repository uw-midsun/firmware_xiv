#include "charger_events.h"
#include "control_pilot.h"
#include "gpio.h"
#include "ms_test_helpers.h"
#include "pwm_input.h"
#include "test_helpers.h"

static uint32_t s_duty_cycle;

StatusCode TEST_MOCK(pwm_input_get_reading)(PwmTimer timer, PwmInputReading *reading) {
  PwmInputReading read = { .dc_percent = s_duty_cycle, .period_us = 0 };
  *reading = read;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  TEST_ASSERT_OK(control_pilot_init());
}

void teardown_test(void) {}

void test_various_duty_cycles(void) {
  // numbers chosen arbitrarily but near boundaries
  uint32_t low_no_charge_allowed_dc = 10;
  uint32_t max_6A_dc = 97;
  uint32_t dc_times_point_6_dc = 600;
  uint32_t dc_minus_64_times_2_point_5_dc = 860;
  uint32_t max_80A_dc = 961;
  uint32_t high_no_charge_allowed_dc = 970;

  Event e = { 0 };
  uint16_t answer = 0;

  // case 1
  s_duty_cycle = low_no_charge_allowed_dc;
  answer = 0;
  event_raise(CHARGER_CP_EVENT_REQUEST_READING, 0);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  control_pilot_process_event(&e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CP_EVENT_VALUE_AVAILABLE, answer);

  // case 2
  s_duty_cycle = max_6A_dc;
  answer = 60;  // max allowed should be 6.0 amps
  event_raise(CHARGER_CP_EVENT_REQUEST_READING, 0);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  control_pilot_process_event(&e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CP_EVENT_VALUE_AVAILABLE, answer);

  // case 3
  s_duty_cycle = dc_times_point_6_dc;
  answer = 360;  // 600 * 0.6 = 360
  event_raise(CHARGER_CP_EVENT_REQUEST_READING, 0);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  control_pilot_process_event(&e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CP_EVENT_VALUE_AVAILABLE, answer);

  // case 4
  s_duty_cycle = dc_minus_64_times_2_point_5_dc;
  answer = 550;  // (860 - 640) * 2.5 = 550
  event_raise(CHARGER_CP_EVENT_REQUEST_READING, 0);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  control_pilot_process_event(&e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CP_EVENT_VALUE_AVAILABLE, answer);

  // case 5
  s_duty_cycle = max_80A_dc;
  answer = 800;
  event_raise(CHARGER_CP_EVENT_REQUEST_READING, 0);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  control_pilot_process_event(&e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CP_EVENT_VALUE_AVAILABLE, answer);

  // case 6
  s_duty_cycle = high_no_charge_allowed_dc;
  answer = 0;
  event_raise(CHARGER_CP_EVENT_REQUEST_READING, 0);
  MS_TEST_HELPER_AWAIT_EVENT(e);
  control_pilot_process_event(&e);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CP_EVENT_VALUE_AVAILABLE, answer);
}
