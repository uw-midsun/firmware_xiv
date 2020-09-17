#include <stdint.h>  // for integer types

#include "interrupt.h"  // interrupts are required for soft timers
#include "log.h"
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "wait.h"        // for wait function

#define COUNTER_A_DELAY_MS 500
#define COUNTER_B_DELAY_MS 1000

typedef struct Counter {
  uint8_t counter_a;
  uint8_t counter_b;
} Counter;

static void prv_counter_a_callback(SoftTimerId timer_id, void *context);
static void prv_counter_b_callback(SoftTimerId timer_id, void *context);

static void prv_counter_a_callback(SoftTimerId timer_id, void *context) {
  Counter *main_counter = context;
  main_counter->counter_a++;

  LOG_DEBUG("Counter A: %d\n", main_counter->counter_a);

  soft_timer_start_millis(COUNTER_A_DELAY_MS, prv_counter_b_callback, main_counter, NULL);
}

static void prv_counter_b_callback(SoftTimerId timer_id, void *context) {
  Counter *main_counter = context;
  main_counter->counter_a++;
  main_counter->counter_b++;

  LOG_DEBUG("Counter A: %d\n", main_counter->counter_a);
  LOG_DEBUG("Counter B: %d\n", main_counter->counter_b);

  soft_timer_start_millis(COUNTER_B_DELAY_MS, prv_counter_a_callback, main_counter, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  Counter main_counter = { .counter_a = 0, .counter_b = 0 };

  soft_timer_start_millis(COUNTER_A_DELAY_MS, prv_counter_a_callback, &main_counter, NULL);

  while (true) {
    wait();
  }

  return 0;
}
