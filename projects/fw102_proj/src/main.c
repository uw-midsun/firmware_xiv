#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "interrupt.h" // interrupts are required for soft timers
#include "soft_timer.h" // for soft timers
#include "log.h" // for printing
#include "wait.h" // for wait function
typedef struct {
    uint8_t counter_a, counter_b;

} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context; //cast struct to void*
  (counters->counter_a)++;
  

  // log output
  LOG_DEBUG("Counter A: %i\n", counters->counter_a);

  if(counters->counter_a % 2 == 0) {
    (counters->counter_b)++;
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  // start the timer again, so it keeps periodically flipping coins
  soft_timer_start_millis(500, prv_timer_callback, counters, NULL);
}

int main() {
  
  interrupt_init(); // interrupts must be initialized for soft timers to work
  soft_timer_init(); // soft timers must be initialized before using them
  Counters counters = {0,0};
  soft_timer_start_millis(500, // timer duration
                          prv_timer_callback, // function to call after timer
                          &counters, // automatically gets cast to void*
                          NULL); // timer id - not needed here
  
  while (true) {
    wait(); // waits until an interrupt is triggered rather than endlessly spinning
  }
  
  return 0;
}