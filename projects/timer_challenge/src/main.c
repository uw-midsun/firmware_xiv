#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void counter_increment(SoftTimerId timer_id, void *context) {
  Counters *counter = context;

  counter->counter_a++;
  LOG_DEBUG("Counter A: %i\n", counter->counter_a);

  if (counter->counter_a % 2 == 0) {
    counter->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counter->counter_b);
  }

  soft_timer_start_millis(500, counter_increment, counter, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counter = { 0 };

  soft_timer_start_millis(500, counter_increment, &counter, NULL);

  while (true) {
    wait();
  }

  return 0;
}
