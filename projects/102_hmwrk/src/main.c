#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_A_PERIOD 500

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void counter_a_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counter = context;

  if (counter->counter_a % 2 == 0 && counter->counter_a != 0) {
    counter->counter_b++;
    LOG_DEBUG("Counter b value:%i\n", counter->counter_b);
  }

  counter->counter_a++;
  LOG_DEBUG("Counter a value:%i\n", counter->counter_a);

  soft_timer_start_millis(COUNTER_A_PERIOD, counter_a_timer_callback, counter, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counter = { 0 };
  soft_timer_start_millis(COUNTER_A_PERIOD, counter_a_timer_callback, &counter, NULL);

  while (true) {
    wait();
  }

  return 0;
}
