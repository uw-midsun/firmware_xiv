#include "steering_can.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can.h"
#include "can_msg_defs.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_digital_input.h"
#include "test_helpers.h"
#include "wait.h"

void setup_test(void) {
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  steering_digital_input_init();
}

void test_steering_digital_input_horn() {
  GpioAddress *horn_address = test_get_address(STEERING_INPUT_HORN_EVENT);
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(horn_address));

  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(STEERING_INPUT_HORN_EVENT, e.id);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, e.data);
  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
void test_steering_digital_input_high_beam_forward() {
  GpioAddress *high_beam_forward_address = test_get_address(STEERING_HIGH_BEAM_FORWARD_EVENT);
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(high_beam_forward_address));

  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(STEERING_HIGH_BEAM_FORWARD_EVENT, e.id);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, e.data);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_steering_digital_input_cc_toggle() {
  GpioAddress *cc_toggle_address = test_get_address(STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT);
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(cc_toggle_address));

  Event e = { 0 };
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT, e.id);
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void teardown_test(void) {}
