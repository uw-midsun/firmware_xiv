#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#include <stdint.h>
#include <stdlib.h>

// Values are in milliseconds.
#define COUNTER_A_DELAY 500
#define COUNTER_B_DELAY 1000

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_a_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;
  counters->counter_a++;
  LOG_DEBUG("Counter A: %d\n", counters->counter_a);
  soft_timer_start_millis(COUNTER_A_DELAY, prv_timer_a_callback, counters, NULL);
}

void prv_timer_b_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;
  counters->counter_b++;
  LOG_DEBUG("Counter B: %d\n", counters->counter_b);
  soft_timer_start_millis(COUNTER_B_DELAY, prv_timer_b_callback, counters, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };

  soft_timer_start_millis(COUNTER_A_DELAY, prv_timer_a_callback, &counters, NULL);
  soft_timer_start_millis(COUNTER_B_DELAY, prv_timer_b_callback, &counters, NULL);

  while (true) {
  }
}