#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "digital_input.h"
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
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"

static CanStorage storage = { 0 };
const CanSettings settings = {
  .device_id = SYSTEM_CAN_DEVICE_DRIVER_CONTROLS_STEERING,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = STEERING_DIGITAL_INPUT_CAN_RX,
  .tx_event = STEERING_DIGITAL_INPUT_CAN_TX,
  .tx = { GPIO_PORT_A, 11 },
  .rx = { GPIO_PORT_A, 12 },
  .loopback = false,
};

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  steering_digital_input_init();
  can_init(&storage,&settings);
}

void teardown_test(void) {}

void test_steering_digital_input_can_horn() {

  // Triggers interrupt for the horn
  GpioAddress pin_horn = { .port = GPIO_PORT_B, .pin = 0 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_horn));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.id, EE_STEERING_INPUT_HORN_PRESSED);

  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
 CanMessage msg = {
    .msg_id = 0x1,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 1,
    .data = 1,
  };
  can_transmit(&msg, NULL);
}

void test_steering_digital_input_can_high_beam_forward() {
  // Triggers interrupt for the high beam in the front
  GpioAddress pin_high_beam_forward = { .port = GPIO_PORT_B, .pin = 3 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_high_beam_forward));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.id, STEERING_DIGITAL_INPUT_HIGH_BEAM_FORWARD);

  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}

void test_steering_digital_input_can_cc_incease_speed() {
  // Triggers interrupt to toggle the cruise control
  GpioAddress pin_cc_toggle = { .port = GPIO_PORT_B, .pin = 7 };
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&pin_cc_toggle));

  Event e = { 0 };
  // Pop item off of queue
  TEST_ASSERT_OK(event_process(&e));
  TEST_ASSERT_EQUAL(e.id, STEERING_DIGITAL_DIGITAL_INPUT_CC_INCREASE_SPEED);

  // Should be empty after the event is popped off
  TEST_ASSERT_EQUAL(STATUS_CODE_EMPTY, event_process(&e));
}
