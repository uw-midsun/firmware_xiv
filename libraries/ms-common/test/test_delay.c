#include "delay.h"

#include "interrupt.h"
#include "soft_timer.h"
#include "unity.h"

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
