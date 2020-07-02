#include "adc.h"
#include "charger_events.h"
#include "connection_sense.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"

// shorten period for faster tests
#undef CONNECTION_SENSE_POLL_PERIOD_MS
#define CONNECTION_SENSE_POLL_PERIOD_MS 10

#define CS_UNPLUGGED_VAL 2400
#define CS_PLUGGED_RELEASED_VAL 1000
#define CS_PLUGGED_PRESSED_VAL 1700
#define CS_FLOATING_VAL 200

static uint16_t s_mock_reading;

void TEST_MOCK(adc_read_converted)(AdcChannel chan, uint16_t *reading) {
  *reading = s_mock_reading;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  TEST_ASSERT_OK(connection_sense_init());
}

void teardown_test(void) {}

void test_state_changes(void) {
  Event e = { 0 };

  // starts unplugged
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // goto plugged pressed
  s_mock_reading = CS_PLUGGED_PRESSED_VAL;
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // goto plugged released
  s_mock_reading = CS_PLUGGED_RELEASED_VAL;
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CHARGE_EVENT_BEGIN, 0);

  // goto unplugged
  s_mock_reading = CS_UNPLUGGED_VAL;
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CHARGE_EVENT_STOP, 0);
}

void test_floating_value(void) {
  Event e = { 0 };
  // starts unplugged
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // goto floating
  // should still act as unplugged
  s_mock_reading = CS_FLOATING_VAL;
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // goto plugged released
  s_mock_reading = CS_PLUGGED_RELEASED_VAL;
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CHARGE_EVENT_BEGIN, 0);

  // goto floating
  s_mock_reading = CS_FLOATING_VAL;
  delay_ms(2 * CONNECTION_SENSE_POLL_PERIOD_MS);
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_CHARGE_EVENT_STOP, 0);
}
