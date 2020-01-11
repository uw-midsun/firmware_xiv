#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "digital_input_config.h"
#include "digital_input_events.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"

static CanStorage storage = { 0 };

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  steering_can_init();
  steering_digital_input_init();
}

void teardown_test(void) {}

void test_steering_digital_input_can_horn() {
  // Triggers interrupt for the horn
  GpioAddress pin_horn = { .port = GPIO_PORT_B, .pin = 0 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_horn));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_OK(can_process_event(&e));

  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_steering_digital_input_can_high_beam_forward() {
  // Triggers interrupt for the high beam in the front
  GpioAddress pin_high_beam_forward = { .port = GPIO_PORT_B, .pin = 1 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_high_beam_forward));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_OK(can_process_event(&e));

  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_steering_digital_input_can_cc_toggle() {
  // Triggers interrupt to toggle the cruise control
  GpioAddress pin_cc_toggle = { .port = GPIO_PORT_B, .pin = 6 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_cc_toggle));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_OK(can_process_event(&e));

  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
