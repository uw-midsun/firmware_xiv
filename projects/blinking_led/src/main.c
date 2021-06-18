#include "interrupt.h"   // apparently soft timers needs this
#include "log.h"         // similar to prinf but indicates the line #
#include "soft_timer.h"  // soft timers
#include "wait.h"        // for wait function

#define COUNTER_PERIOD_MS 500  // the time between each count in ms

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_SoftTimerCallCounter(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);
  if ((storage->counter_a) % 2 == 0) {
    storage->counter_b++;
    LOG_DEBUG("Counter B: %i\n", storage->counter_b);
  }
  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_SoftTimerCallCounter, storage, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();
  Counters storage = { 0 };
  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_SoftTimerCallCounter, &storage, NULL);

  while (true) {
    wait();
  }
  return 0;
}
