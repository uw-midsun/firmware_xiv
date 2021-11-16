#include <stdint.h>
#include <stdlib.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_A_WAIT_PERIOD_MS 500
#define COUNTER_B_WAIT_PERIOD_MS 1000

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback_a(SoftTimerId timer_id, void *context) {
  Counters *counters = context;
  counters->counter_a++;
  LOG_DEBUG("COUNTER A: %i\n", counters->counter_a);

  soft_timer_start_millis(COUNTER_A_WAIT_PERIOD_MS, prv_timer_callback_a, counters, NULL);
}

void prv_timer_callback_b(SoftTimerId timer_id, void *context) {
  Counters *counters = context;
  counters->counter_b++;
  LOG_DEBUG("COUNTER B: %i\n", counters->counter_b);

  soft_timer_start_millis(COUNTER_B_WAIT_PERIOD_MS, prv_timer_callback_b, counters, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };

  soft_timer_start_millis(COUNTER_A_WAIT_PERIOD_MS, prv_timer_callback_a, &counters, NULL);

  soft_timer_start_millis(COUNTER_B_WAIT_PERIOD_MS, prv_timer_callback_b, &counters, NULL);

  while (true) {
    wait();
  }

  return 0;
}
