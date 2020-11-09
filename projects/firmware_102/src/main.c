#include <stdbool.h>
#include <stdint.h>  // for integer types
#include <stdlib.h>  // for random numbers
#include "interrupt.h"
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *count = context;
  count->counter_a += 1;
  LOG_DEBUG("Counter A:%d\n", count->counter_a);

  if (count->counter_a % 2 == 0 && count->counter_a != 0) {
    count->counter_b += 1;
    LOG_DEBUG("Counter B:%d\n", count->counter_b);
  }

  soft_timer_start_millis(500,                 // timer duration
                          prv_timer_callback,  // function to call after timer
                          count,               // automatically gets cast to void*
                          NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters count = { 0 };

  soft_timer_start_millis(500,                 // timer duration
                          prv_timer_callback,  // function to call after timer
                          &count,              // automatically gets cast to void*
                          NULL);

  while (true) {
    wait();
  }
}
