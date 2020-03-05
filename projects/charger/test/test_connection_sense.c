#include "event_queue.h"
#include "gpio.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"

#include "charger_connection_sense.h"
#include "charger_events.h"

void setup_test(void) {
  event_queue_init();
  gpio_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_init() {
  // Tests init of timer reading ADC
  TEST_ASSERT_OK(connection_sense_init());
}
