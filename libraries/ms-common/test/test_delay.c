#include "delay.h"

#include "interrupt.h"
#include "soft_timer.h"
#include "unity.h"

bool soft_timer_done;

// These tests serve a dual purpose as they also implicitly test the wait
// module.

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
}

void teardown_test(void) {}

void test_delay_us(void) {
  delay_us(10000);
}

// The following 2 functions test that delay_us can be used
// inside a soft timer callback without deadlock (issue SOFT-298)
static void callback(SoftTimerId timer, void *context) {
  delay_us(10000);
  soft_timer_done = true;
}
void test_delay_in_soft_timer(void) {
  soft_timer_done = false;
  soft_timer_start(10000, callback, NULL, NULL);

  // Soft timer should be done after 20000 us
  delay_us(20000);
  TEST_ASSERT_TRUE(soft_timer_done);
}
