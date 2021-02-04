#include <stdint.h>  // for integer types

#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

#define COUNTER_A_INCREMENT_INTERVAL_MS 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  // casting void* back to our struct so we can use it outside of main()
  Counters *p_counter = context;

  p_counter->counter_a++;
  LOG_DEBUG("Counter A: %i \n", p_counter->counter_a);

  // if counter_a is even, a multiple of 1000ms has passed and counter_b should be incremented
  if (p_counter->counter_a % 2 == 0) {
    p_counter->counter_b++;
    LOG_DEBUG("Counter B: %i \n", p_counter->counter_b);
  }

  // restart timer to continue clock cycle
  soft_timer_start_millis(COUNTER_A_INCREMENT_INTERVAL_MS, prv_timer_callback, p_counter, NULL);
}

int main(void) {
  interrupt_init();  // must have this for soft_timer
  soft_timer_init();

  Counters counter = { 0 };

  soft_timer_start_millis(COUNTER_A_INCREMENT_INTERVAL_MS,  // timer duration
                          prv_timer_callback,               // function to call after timer
                          &counter,                         // automatically cast to void
                          NULL);                            // timer id - not needed
  while (true) {
    wait();
  }

  return 0;
}
