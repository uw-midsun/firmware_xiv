#include <stdint.h>
#include <stdlib.h>
#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

#define INTERVAL_COUNTER_A 500
#define INTERVAL_COUNTER_B 1000

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Main_var;

void timer_callback_A(SoftTimerId timer_id, void *context) {
  Main_var *counters = context;
  counters->counter_a++;

  LOG_DEBUG("Counter A : %i\n", counters->counter_a);

  soft_timer_start_millis(INTERVAL_COUNTER_A, timer_callback_A, counters, NULL);
}

void timer_callback_B(SoftTimerId timer_id, void *context) {
  Main_var *counters = context;
  counters->counter_b++;

  LOG_DEBUG("Counter B : %i", counters->counter_b);
  soft_timer_start_millis(INTERVAL_COUNTER_B, timer_callback_B, counters, NULL);
}
int main(void) {
  interrupt_init();
  soft_timer_init();
  Main_var counters = { 0 };
  soft_timer_start_millis(INTERVAL_COUNTER_A, timer_callback_A, &counters, NULL);
  soft_timer_start_millis(INTERVAL_COUNTER_B, timer_callback_B, &counters, NULL);

  while (true) {
    wait();
  }

  return 0;
}
