#include <stdint.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
#define INTERVAL 0.5

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;
void prv_callback(SoftTimerId id, void *context) {
  Counters *counter = context;
  counter->counter_a++;
  LOG_DEBUG("Counter A: %i\n", counter->counter_a);
  if (counter->counter_a % 2 == 0) {
    counter->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counter->counter_b);
  }
  soft_timer_start_seconds(INTERVAL, prv_callback, counter, NULL);
}

int main(void) {
  Counters counter = { 0 };
  interrupt_init();
  soft_timer_init();
  soft_timer_start_seconds(INTERVAL, prv_callback, &counter, NULL);
  while (true) {
    wait();
  }
  return 0;
}
