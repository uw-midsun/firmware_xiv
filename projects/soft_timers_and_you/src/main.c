#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#include <stdint.h>

#define CONST_MILLIS_COUNTER 500

typedef struct {
  uint8_t counter_a; 
  uint8_t counter_b;
} Counters;

static void prv_counter(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->counter_a++;
  if (counter->counter_a % 2 == 0) {
    LOG_DEBUG("Counter A: %i\n", counter->counter_a);
    counter->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counter->counter_b);
    soft_timer_start_millis(CONST_MILLIS_COUNTER, prv_counter, counter, NULL);
  } else {
    LOG_DEBUG("Counter A: %i\n", counter->counter_a);
    soft_timer_start_millis(CONST_MILLIS_COUNTER, prv_counter, counter, NULL);
  }
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };
  soft_timer_start_millis(CONST_MILLIS_COUNTER, prv_counter, &counters, NULL);

  while (true) {
    wait();
  }

  return 0;
}
