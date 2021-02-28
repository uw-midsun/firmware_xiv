#include <stdint.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define TIMER_PERIOD_MS 500  // timer in ms

typedef struct Counters {  // stores counters
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;

  counters->counter_a++;  // increments a every 0.5s

  LOG_DEBUG("Counter A: %i\n", counters->counter_a);

  if (counters->counter_a % 2 == 0) {  // increments b every other cycle
    counters->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  // restart timer again after
  soft_timer_start_millis(TIMER_PERIOD_MS, prv_timer_callback, counters, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };

  soft_timer_start_millis(TIMER_PERIOD_MS, prv_timer_callback, &counters, NULL);
  while (true) {
    wait();
  }

  return 0;
}
