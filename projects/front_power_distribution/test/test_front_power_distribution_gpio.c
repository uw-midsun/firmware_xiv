#include "front_power_distribution_events.h"
#include "front_power_distribution_gpio.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

static void prv_test_turn_on(EventId id, GpioAddress *address) {
  Event e = { .id = id, .data = 1 };
  StatusCode code = front_power_distribution_gpio_process_event(&e);
  TEST_ASSERT(code == STATUS_CODE_OK);

  GpioState new_state;
  gpio_get_state(address, &new_state);
  TEST_ASSERT(new_state == GPIO_STATE_HIGH);
}

void setup_test(void) {
  front_power_distribution_gpio_init();
}
void teardown_test(void) {}

void test_front_power_distribution_gpio_each_pin_on(void) {
  GpioAddress *gpio_addresses = front_power_distribution_gpio_test_provide_gpio_addresses();
  FrontPowerDistributionGpioOutput *events_to_outputs =
      front_power_distribution_gpio_test_provide_events_to_outputs();

  // perform the test for every event except SIGNAL_HAZARD which is handled separately
  for (FrontPowerDistributionGpioEvent id = FRONT_POWER_DISTRIBUTION_GPIO_EVENT_DRIVER_DISPLAY;
       id < FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD; id++) {
    prv_test_turn_on(id, &gpio_addresses[events_to_outputs[id]]);
  }
}

void test_front_power_distribution_gpio_signal_hazard_on(void) {
  GpioAddress *gpio_addresses = front_power_distribution_gpio_test_provide_gpio_addresses();
  FrontPowerDistributionGpioOutput *events_to_outputs =
      front_power_distribution_gpio_test_provide_events_to_outputs();

  Event e = { .id = FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD, .data = 1 };
  StatusCode code = front_power_distribution_gpio_process_event(&e);
  TEST_ASSERT(code == STATUS_CODE_OK);

  GpioState new_state;
  gpio_get_state(
      &gpio_addresses[events_to_outputs[FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT]],
      &new_state);
  TEST_ASSERT(new_state == GPIO_STATE_HIGH);
  gpio_get_state(
      &gpio_addresses[events_to_outputs[FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT]],
      &new_state);
  TEST_ASSERT(new_state == GPIO_STATE_HIGH);
}

void test_front_power_distribution_gpio_invalid_id(void) {
  Event e = { .id = 0xABCD, .data = 1 };
  StatusCode code = front_power_distribution_gpio_process_event(&e);
  TEST_ASSERT(code == STATUS_CODE_OUT_OF_RANGE);
}
