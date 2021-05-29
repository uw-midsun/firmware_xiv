#include <stdint.h>
#include <stdlib.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
#define HALF_SEC 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_counter_increment(SoftTimerId id, void *context) {
  Counters *counters = context;

  counters->counter_a += 1;
  LOG_DEBUG("Counter A: %i\n", counters->counter_a);

  if (counters->counter_a % 2 == 0) {
    counters->counter_b += 1;
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  soft_timer_start_millis(HALF_SEC, prv_counter_increment, counters, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();
  Counters counters = { 0 };
  soft_timer_start_millis(HALF_SEC, prv_counter_increment, &counters, NULL);
  while (true) {
    wait();
  }
  return 0;
}
