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

  static SteeringDigitalInputConfiguration s_steering_lookup_table[NUM_STEERING_DIGITAL_INPUTS] = {
    [STEERING_DIGITAL_INPUT_HORN] = { .event = STEERING_DIGITAL_INPUT_EVENT_HORN,
                                      .address = { .port = GPIO_PORT_B, .pin = 0 } },
    [STEERING_DIGITAL_INPUT_RADIO_PPT] = { .event = STEERING_DIGITAL_INPUT_EVENT_RADIO_PPT,
                                           .address = { .port = GPIO_PORT_B, .pin = 1 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD] =
        { .event = STEERING_DIGITAL_INPUT_EVENT_HIGH_BEAM_FORWARD,
          .address = { .port = GPIO_PORT_B, .pin = 3 } },
    [STEERING_DIGITAL_INPUT_HIGH_BEAM_REAR] = { .event =
                                                    STEERING_DIGITAL_INPUT_EVENT_HIGH_BEAM_REAR,
                                                .address = { .port = GPIO_PORT_B, .pin = 4 } },
    [STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE] =
        { .event = STEERING_DIGITAL_INPUT_EVENT_REGEN_BRAKE_TOGGLE,
          .address = { .port = GPIO_PORT_B, .pin = 5 } },
    [STEERING_DIGITAL_INPUT_CC_TOGGLE] = { .event = STEERING_DIGITAL_INPUT_EVENT_CC_TOGGLE,
                                           .address = { .port = GPIO_PORT_B, .pin = 6 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED] =
        { .event = STEERING_DIGITAL_DIGITAL_INPUT_EVENT_CC_INCREASE_SPEED,
          .address = { .port = GPIO_PORT_B, .pin = 7 } },
    [STEERING_DIGITAL_DIGITAL_INPUT_CC_DECREASE_SPEED] =
        { .event = STEERING_DIGITAL_DIGITAL_INPUT_EVENT_CC_DECREASE_SPEED,
          .address = { .port = GPIO_PORT_B, .pin = 8 } },
  };

  steering_can_init();
  steering_digital_input_init(s_steering_lookup_table);
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
