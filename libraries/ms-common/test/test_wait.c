#include "wait.h"

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

static int s_num_wait_cycles_timer;
static int s_num_wait_cycles_gpio;

#define WAIT_INTERVAL_S 2
#define EXPECTED_INTERRUPT_CYCLES 2

static void prv_test_wait_interrupt_callback(SoftTimerId id, void *context) {
  s_num_wait_cycles_timer++;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  s_num_wait_cycles_timer = 0;
}

void teardown_test(void) {}

void test_wait_init_works_timer(void) {
  int done = 0;
  while (!done) {
    soft_timer_start_seconds(WAIT_INTERVAL_S, prv_test_wait_interrupt_callback, NULL, NULL);

    wait();
    s_num_wait_cycles_timer++;
    if (s_num_wait_cycles_timer > EXPECTED_INTERRUPT_CYCLES - 1) {
      break;
    }
  }
  TEST_ASSERT_EQUAL(EXPECTED_INTERRUPT_CYCLES, s_num_wait_cycles_timer);
}
