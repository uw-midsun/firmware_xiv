#include "charger_events.h"
#include "gpio.h"
#include "ms_test_helpers.h"
#include "status.h"
#include "stop_sequence.h"
#include "test_helpers.h"

static uint8_t s_gpio_set_state_calls;
static uint8_t s_charger_deactivate_calls;

#define NUM_GPIO_SET_CALLS_IN_STOP_SEQUENCE 3

StatusCode TEST_MOCK(charger_controller_deactivate)() {
  s_charger_deactivate_calls++;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(gpio_set_state)(const GpioAddress *address, GpioState state) {
  s_gpio_set_state_calls++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  s_gpio_set_state_calls = 0;
  s_charger_deactivate_calls = 0;
  event_queue_init();
}

void teardown_test(void) {}

void test_stop_sequence(void) {
  event_raise(CHARGER_CHARGE_EVENT_STOP, 0);

  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);

  TEST_ASSERT(s_charger_deactivate_calls == 0);
  TEST_ASSERT(s_gpio_set_state_calls == 0);

  stop_sequence_process_event(&e);

  TEST_ASSERT(s_charger_deactivate_calls == 1);
  TEST_ASSERT(s_gpio_set_state_calls == NUM_GPIO_SET_CALLS_IN_STOP_SEQUENCE);
}
