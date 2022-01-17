#include <stdlib.h>

#include "interrupt.h"  // interrupts are required for soft timers
#include "log.h"
#include "soft_timer.h"  // for soft timers

#define COUNTER_A_DELAY 500  // ms

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void incrementCounter(SoftTimerId timer_id, void *context) {
  Counters *counters = context;  // cast void* to our struct so we can use it

  LOG_DEBUG("Counter A: %i\n", counters->counter_a);
  if (counters->counter_a % 2 == 0) {
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
    counters->counter_b++;
  }
  counters->counter_a++;

  soft_timer_start_millis(COUNTER_A_DELAY, incrementCounter, counters, NULL);
}

int main() {
  interrupt_init();   // interrupts must be initialized for soft timers to work
  soft_timer_init();  // soft timers must be initialized before using them

  Counters counters = { .counter_a = 1, .counter_b = 1 };

  soft_timer_start_millis(COUNTER_A_DELAY, incrementCounter, &counters, NULL);

  while (1) {
  }
  return 0;
}
