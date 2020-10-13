#include <stdint.h>  // for integer types
#include <stdlib.h>  // for random numbers

#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;  // cast void* to our struct so we can use it

  // always increment and log counter_a
  counters->counter_a++;
  LOG_DEBUG("Counter A: %i\n", counters->counter_a);

  // increment and log counter_b every other call
  if (counters->counter_a % 2 == 0) {
    counters->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  // start the timer again
  soft_timer_start_millis(500, prv_timer_callback, counters, NULL);
}

int main(void) {
  interrupt_init();   // interrupts must be initialized for soft timers to work
  soft_timer_init();  // soft timers must be initialized before using them

  // initialize the Counters struct
  Counters counters = { .counter_a = 0, .counter_b = 0 };

  // start counting
  soft_timer_start_millis(500,                 // half-second duration
                          prv_timer_callback,  // function to call after timer
                          &counters,           // automatically gets cast to void*
                          NULL);               // timer id - not needed here

  while (true) {
    wait();
  }

  return 0;
}
