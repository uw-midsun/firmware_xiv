#include <stdint.h>  // for integer types

#include "delay.h"       // for delays
#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);
  if (storage->counter_a % 2 == 0) {  // for every two increments in Counter A, counter_b++
    storage->counter_b++;
    LOG_DEBUG("Counter B: %i\n", storage->counter_b);
  }

  soft_timer_start_millis(500, prv_timer_callback, storage, NULL);  // restart timer
}

int main() {
  interrupt_init();   // interrupts need to be initialized for soft timers to work
  soft_timer_init();  // soft timers need to be initialized before use

  Counters storage = { 0 };  // initialize struct to be all 0

  soft_timer_start_millis(500, prv_timer_callback, &storage, NULL);

  while (true) {
    wait();
  }

  return 0;
}
