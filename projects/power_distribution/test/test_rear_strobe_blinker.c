#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pd_events.h"
#include "rear_strobe_blinker.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_BLINK_DELAY_US 20000

static const Event s_start_event = { .id = POWER_DISTRIBUTION_STROBE_EVENT, .data = 1 };
static const Event s_stop_event = { .id = POWER_DISTRIBUTION_STROBE_EVENT, .data = 0 };

void setup_test() {
  interrupt_init();
  soft_timer_init();
  event_queue_init();
}
void teardown_test() {}

// Comprehensive happy-path test - we can turn it on and off with the correct events
void test_rear_power_distribution_strobe_blinker_works(void) {
  RearPowerDistributionStrobeBlinkerSettings settings = {
    .strobe_blink_delay_us = TEST_BLINK_DELAY_US,
  };
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_init(&settings));

  // It doesn't start blinking until we start it
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_us(2 * TEST_BLINK_DELAY_US);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // It starts blinking at the correct delay when we start it
  Event e = { 0 };
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&s_start_event));
  // starts immediately, initial is on
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 1);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_DELAY_US);
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 0);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_DELAY_US);
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 1);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // It stops blinking when we stop it
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&s_stop_event));
  // emits a final event to bring back to off
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 0);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_us(2 * TEST_BLINK_DELAY_US);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // We can restart it and stop immediately
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&s_start_event));
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 1);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&s_stop_event));
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 0);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_us(2 * TEST_BLINK_DELAY_US);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that the strobe blinker interprets any nonzero data as on.
void test_rear_power_distribution_strobe_blinker_coalesces_nonzero_to_on(void) {
  RearPowerDistributionStrobeBlinkerSettings settings = {
    .strobe_blink_delay_us = TEST_BLINK_DELAY_US,
  };
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_init(&settings));

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  Event weird_start_event = { .id = POWER_DISTRIBUTION_STROBE_EVENT, .data = 0x80 };
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&weird_start_event));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 1);

  // stop it
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&s_stop_event));
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 0);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// Test that the strobe blinker ignores events other than POWER_DISTRIBUTION_STROBE_EVENT.
void test_rear_power_distribution_strobe_blinker_ignores_other_events(void) {
  RearPowerDistributionStrobeBlinkerSettings settings = {
    .strobe_blink_delay_us = TEST_BLINK_DELAY_US,
  };
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_init(&settings));

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // doesn't start it
  Event other_event = { .id = POWER_DISTRIBUTION_STROBE_EVENT + 1, .data = 1 };
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&other_event));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_us(2 * TEST_BLINK_DELAY_US);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&s_start_event));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 1);

  // doesn't stop it
  Event other_event2 = { .id = POWER_DISTRIBUTION_STROBE_EVENT + 1, .data = 0 };
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&other_event2));
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  delay_us(TEST_BLINK_DELAY_US);
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_GPIO_EVENT_STROBE, 0);

  // stop it
  TEST_ASSERT_OK(rear_power_distribution_strobe_blinker_process_event(&s_stop_event));
  // no final event because it ended on a 0
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}
