#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#include <stdint.h>

typedef struct Counters {
  uint8_t counter_a, counter_b;
} Counters_t;

void run(SoftTimerId timer_id, void *ctx) {
  Counters_t *counter = (Counters_t *)ctx;

  counter->counter_a++;
  LOG_DEBUG("Counter A: %u\n", counter->counter_a);
  if (!(counter->counter_a % 2)) {
    counter->counter_b++;
    LOG_DEBUG("Counter B: %u\n", counter->counter_b);
  }

  soft_timer_start_millis(500, run, counter, NULL);
}

int main(int argc, const char **argv) {
  Counters_t counter = { 0 };

  interrupt_init();
  soft_timer_init();

  run(0, &counter);

  while (1) {
    wait();
  }

  return 0;
}
