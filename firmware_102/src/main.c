#include <stdint.h>
#include <stdlib.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
#define COUNTERS_PERIOD_MS 1000

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void counter_logger(SoftTimerId(timer_id), void *context) {
  Counters *storage = context;
  storage->counter_a++;
  LOG_DEBUG("Counter_A: %d\n", storage->counter_a);

  if ((storage->counter_a) % 2 == 0) {
    storage->counter_b++;
    LOG_DEBUG("Counter_B: %d\n", storage->counter_b);
  }
  soft_timer_start_millis(COUNTERS_PERIOD_MS, counter_logger, storage, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters storage = { 0 };

  soft_timer_start_millis(COUNTERS_PERIOD_MS, counter_logger, &storage, NULL);

  while (true) {
    wait();
  }

  return 0;
}
