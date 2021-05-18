#include <stdint.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define DELAY_MS 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_increment_counters(SoftTimerId id, void *context) {
  Counters *counters = context;

  counter->counter_a++;
  LOG_DEBUG("Counter A: %i\n", counter->counter_a);
  if (counter->counter_a % 2 == 0) {
    counter->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counter->counter_b);
  }

  soft_timer_start_millis(DELAY_MS, prv_increment_counters, context, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };

  soft_timer_start_millis(DELAY_MS, prv_increment_counters, &counter, NULL);
  while (true) {
    wait();
  }
  return 0;
}
