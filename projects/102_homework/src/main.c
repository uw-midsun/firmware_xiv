#include <stdint.h>
#include <stdlib.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define DELAY_COUNTER_A_MS 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void soft_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *increment = context;
  increment->counter_a++;
  LOG_DEBUG("Counter A: %i\n", increment->counter_a);

  if (increment->counter_a % 2 == 0) {
    increment->counter_b++;
    LOG_DEBUG("Counter B: %i\n", increment->counter_b);
  }
  soft_timer_start_millis(DELAY_COUNTER_A_MS, soft_timer_callback, increment, NULL);
}

int main(void) {
  Counters storage = { 0 };

  interrupt_init();
  soft_timer_init();
  soft_timer_start_millis(DELAY_COUNTER_A_MS, soft_timer_callback, &storage, NULL);

  while (true) {
    wait();
  }
  return 0;
}
