#include <stdbool.h>
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timing_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;

  counters->counter_a++;  // increments a

  LOG_DEBUG("Counter A: %i\n", counters->counter_a);

  if (counters->counter_a % 2 == 0) {  // increments b
    counters->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  // starts timer again
  soft_timer_start_millis(500, prv_timing_callback, counters, NULL);
}

int main(void) {
  LOG_DEBUG("Timer FW102 HW\n");

  // init interrupt and timer
  interrupt_init();
  soft_timer_init();

  // initialize the struct to be all 0
  Counters counters = { 0 };

  soft_timer_start_millis(500, prv_timing_callback, &counters, NULL);

  while (true) {
    wait();
  }

  return 0;
}
