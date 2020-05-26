#include "counter.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_TIMEOUT_MS 500

typedef struct {
  uint8_t count_a;
  uint8_t count_b;
} Counters;

void counter_a_timeout(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->count_a++;
  LOG_DEBUG("Counter A: %d\n", counter->count_a);

  soft_timer_start_millis(COUNTER_TIMEOUT_MS, counter_b_timeout, counter, NULL);
}

void counter_b_timeout(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->count_a++;
  counter->count_b++;
  LOG_DEBUG("Counter A: %d\n", counter->count_a);
  LOG_DEBUG("Counter B: %d\n", counter->count_b);

  soft_timer_start_millis(COUNTER_TIMEOUT_MS, counter_a_timeout, counter, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counter = { 0 };
  soft_timer_start_millis(COUNTER_TIMEOUT_MS, counter_a_timeout, &counter, NULL);

  while (true) {
    wait();
  }

  return 0;
}
