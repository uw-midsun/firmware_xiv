#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

#define COUNTER_UPDATE_PERIOD_MS 500  // miliseconds between counter update

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_counter_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);

  if (storage->counter_a % 2 == 0) {
    storage->counter_b++;
    LOG_DEBUG("Counter B: %i\n", storage->counter_b);
  }

  soft_timer_start_millis(COUNTER_UPDATE_PERIOD_MS, prv_counter_timer_callback, storage, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counterStorage = { 0 };

  soft_timer_start_millis(COUNTER_UPDATE_PERIOD_MS, prv_counter_timer_callback, &counterStorage,
                          NULL);

  while (true) {
    wait();
  }

  return 0;
}
