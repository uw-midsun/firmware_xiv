#include <string.h>

#include "centre_console_events.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "race_switch.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static RaceSwitchFsmStorage s_race_switch_fsm_storage;
static GpioState s_returned_state;
static GpioAddress s_race_switch_address = { .port = GPIO_PORT_A, .pin = 4 };
static GpioAddress s_voltage_monitor_address = { .port = GPIO_PORT_B, .pin = 7 };

StatusCode TEST_MOCK(gpio_get_state)(const GpioAddress *address, GpioState *state) {
  *state = s_returned_state;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();
  soft_timer_init();
  memset(&s_race_switch_fsm_storage, 0, sizeof(s_race_switch_fsm_storage));
  TEST_ASSERT_OK(race_switch_fsm_init(&s_race_switch_fsm_storage));
}

void teardown_test(void) {}

void prv_assert_current_race_state(RaceState state) {
  TEST_ASSERT_EQUAL(state, race_switch_fsm_get_current_state(&s_race_switch_fsm_storage));
}

void test_transition_to_race(void) {
  // Test state machine when normal -> race
  // Initially the module begins in normal mode so PA4 is low
  s_returned_state = GPIO_STATE_LOW;
  prv_assert_current_race_state(RACE_STATE_OFF);

  // Mock rising edge on race switch pin
  s_returned_state = GPIO_STATE_HIGH;

  // Trigger interrupt to change fsm state from normal to race
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  prv_assert_current_race_state(RACE_STATE_ON);

  // Test if no error when interrupt is triggered with same edge multiple times
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  prv_assert_current_race_state(RACE_STATE_ON);
}

void test_transition_to_normal(void) {
  // Test state machine when race -> normal
  // Initially the module begins in race mode so PA4 is high
  s_returned_state = GPIO_STATE_HIGH;
  prv_assert_current_race_state(RACE_STATE_ON);

  // Mock falling edge on race switch pin
  s_returned_state = GPIO_STATE_LOW;

  // Trigger interrupt to change fsm state from race to normal
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  prv_assert_current_race_state(RACE_STATE_OFF);
}

void test_voltage_during_transition(void) {
  // Test voltage regulator when normal -> race -> normal
  // Initially the car is in normal mode so the voltage regulator is enabled
  s_returned_state = GPIO_STATE_LOW;
  prv_assert_current_race_state(RACE_STATE_OFF);

  GpioState voltage_monitor_state;
  // Mock high state on voltage monitor pin
  s_returned_state = GPIO_STATE_HIGH;

  gpio_get_state(&s_voltage_monitor_address, &voltage_monitor_state);
  TEST_ASSERT_EQUAL(voltage_monitor_state, s_returned_state);

  // Switch to race mode
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  prv_assert_current_race_state(RACE_STATE_ON);

  // Mock low state on voltage monitor pin
  s_returned_state = GPIO_STATE_LOW;
  gpio_get_state(&s_voltage_monitor_address, &voltage_monitor_state);
  TEST_ASSERT_EQUAL(voltage_monitor_state, s_returned_state);

  // Switch to normal mode
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_race_switch_address));
  prv_assert_current_race_state(RACE_STATE_OFF);

  // Mock high state on voltage monitor pin
  s_returned_state = GPIO_STATE_HIGH;
  gpio_get_state(&s_voltage_monitor_address, &voltage_monitor_state);
  TEST_ASSERT_EQUAL(voltage_monitor_state, s_returned_state);
}
