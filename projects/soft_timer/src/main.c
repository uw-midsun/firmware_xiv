#include <stdint.h>
#include <stdlib.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Values are in milliseconds.
#define COUNTER_A_DELAY 500

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;

  // Increment counter A every period (500ms)
  counters->counter_a++;
  LOG_DEBUG("Counter A: %d\n", counters->counter_a);

  // Increment counter B every other period (1000ms)
  if (counters->counter_a % 2 == 0) {
    counters->counter_b++;
    LOG_DEBUG("Counter B: %d\n", counters->counter_b);
  }

  soft_timer_start_millis(COUNTER_A_DELAY, prv_timer_callback, counters, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };

  soft_timer_start_millis(COUNTER_A_DELAY, prv_timer_callback, &counters, NULL);

  while (true) {
  }
}
