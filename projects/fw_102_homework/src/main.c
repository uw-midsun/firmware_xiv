#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *storage = context;

  ++storage->counter_a;
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);

  if (storage->counter_a % 2 == 0) {
    ++storage->counter_b;
    LOG_DEBUG("Counter B: %i\n", storage->counter_b);
  }

  soft_timer_start_millis(500, prv_timer_callback, storage, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters storage = { 0 };

  soft_timer_start_millis(500, prv_timer_callback, &storage, NULL);

  while (true) {
    wait();
  }

  return 0;
}
