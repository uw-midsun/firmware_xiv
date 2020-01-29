#include "button_press.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "unity.h"

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  event_queue_init();
}

void teardown_test(void) {}

void test_button_press_raises_events(void) {
  // given
  button_press_init();
  GpioAddress *addresses = test_provide_button_addresses();
  GpioAddress power_button = addresses[CENTRE_CONSOLE_BUTTON_POWER];

  // when
  gpio_it_trigger_interrupt(&power_button);

  // then
  Event e = { 0 };
  event_process(&e);
  TEST_ASSERT_EQUAL(CENTRE_CONSOLE_BUTTON_PRESS_EVENT_POWER, e.id);
}
