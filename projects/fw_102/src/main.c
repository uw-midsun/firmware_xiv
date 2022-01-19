#include <stdint.h>  // for integer types

#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

#define COUNTER_PERIOD_MS 500  // ms

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

// Timeout callback
static void prv_counter_timeout(SoftTimerId timer_id, void *context) {
  Counters *counters = context;
  counters->counter_a++;
  LOG_DEBUG("Counter A %i\n", counters->counter_a);
  if (counters->counter_a % 2 == 0) {
    counters->counter_b++;
    LOG_DEBUG("Counter B %i\n", counters->counter_b);
  }

  // Schedule another timer - this creates a periodic timer
  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_counter_timeout, counters, NULL);
}

int main(void) {
  interrupt_init();   // interrupts must be initialized for soft timers to work
  soft_timer_init();  // soft timers must be initialized before using them

  Counters counters = { 0 };

  // Begin a timer
  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_counter_timeout, &counters, NULL);

  while (true) {
    wait();  // wait till interrupt is triggered
  }
  return 0;
}
