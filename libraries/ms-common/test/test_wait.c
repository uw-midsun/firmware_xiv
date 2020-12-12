#include "wait.h"

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static int s_num_wait_cycles_timer;
static int s_num_wait_times_callback_called;

#define WAIT_INTERVAL_S 2
#define EXPECTED_INTERRUPT_CYCLES 2
#define EXPECTED_TIMES_CALLBACK_CALLED 2

static void prv_test_wait_interrupt_callback(SoftTimerId id, void *context) {
  s_num_wait_times_callback_called++;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  s_num_wait_cycles_timer = 0;
  s_num_wait_times_callback_called = 0;
}

void teardown_test(void) {}

void test_wait_works_timer(void) {
  int done = 0;
  while (s_num_wait_times_callback_called < EXPECTED_TIMES_CALLBACK_CALLED) {
    soft_timer_start_seconds(WAIT_INTERVAL_S, prv_test_wait_interrupt_callback, NULL, NULL);

    wait();
    s_num_wait_cycles_timer++;
  }
  TEST_ASSERT_EQUAL(EXPECTED_INTERRUPT_CYCLES, s_num_wait_cycles_timer);
  TEST_ASSERT_EQUAL(EXPECTED_TIMES_CALLBACK_CALLED, s_num_wait_times_callback_called);
}
