#include <stdint.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNT_PERIOD_MS 500

typedef struct Counters {
  uint8_t counter_a, counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *ctx) {
  Counters *counter = (Counters *)ctx;

  counter->counter_a++;
  LOG_DEBUG("Counter A: %u\n", counter->counter_a);
  if (!(counter->counter_a % 2)) {
    counter->counter_b++;
    LOG_DEBUG("Counter B: %u\n", counter->counter_b);
  }

  soft_timer_start_millis(COUNT_PERIOD_MS, prv_timer_callback, counter, NULL);
}

int main(void) {
  Counters counter = { 0 };

  interrupt_init();
  soft_timer_init();

  prv_timer_callback(SOFT_TIMER_INVALID_TIMER, &counter);

  while (1) {
    wait();
  }

  return 0;
}
