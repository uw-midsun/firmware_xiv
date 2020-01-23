#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can.h"
#include "can_msg_defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "delay.h"
#include "digital_input.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "misc.h"
#include "soft_timer.h"
#include "status.h"
#include "wait.h"
#include "log.h"
#include "test_helpers.h"
#include "ms_test_helpers.h"

/*
1) Register interrupt for each pin
2) Poll and wait for item to be pushed on the queue
3) When the intterupt is triggered, send a CAN message that pushes an event onto the queue
   that coorelates
4) The CAN message should coorelate to the values from the exported enum
   eg EE_STEERING_INPUT_CC_SPEED_PLUS_PRESSED
  
  Make a lookup table with where Event is an array
  with a high and low state, this should coorelate to the EE value

  When the interrupt is registered, it should get the state and send 
  a message according to the previous state
  Eg if it was on it should turn off and vice versa

5) Pop event off of queue and read the data
6) Process the event and send to front power distribution to provide power to that part
  of the vehicle


  CURRENT ERRORS:
  LOG_DEBUG does not work when place into the beginning of digital_input_init
  The callback function is not called when an interrupt is triggered
  What number is being processed in the tests?

*/

void setup_test(void) {
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  steering_digital_input_init();
}

void test_steering_digital_input_can_horn() {

  // Triggers interrupt for the horn
  GpioAddress pin_horn = { .port = GPIO_PORT_B, .pin = 1 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_horn));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(EE_STEERING_INPUT_HORN, e.id);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, e.data);
  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));

 
}

void test_steering_digital_input_can_high_beam_forward() {
  // Triggers interrupt for the high beam in the front
  GpioAddress pin_high_beam_forward = { .port = GPIO_PORT_A, .pin = 7 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_high_beam_forward));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(EE_STEERING_HIGH_BEAM_FORWARD, e.id);
  TEST_ASSERT_EQUAL(GPIO_STATE_LOW, e.data);
  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_steering_digital_input_can_cc_toggle() {
  // Triggers interrupt to toggle the cruise control
  GpioAddress pin_cc_toggle = { .port = GPIO_PORT_B, .pin = 5 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_cc_toggle));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(EE_STEERING_INPUT_CC_TOGGLE_PRESSED, e.id);
  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}


void teardown_test(void) {}
