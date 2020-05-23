#include "counter.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_TIMEOUT_MILI_SECS 500

typedef struct {
  uint8_t count_a;
  uint8_t count_b;
} Counters;

static void counter_a_timeout(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->count_a++;
  LOG_DEBUG("Counter A: %d\n", counter->count_a);

  soft_timer_start_millis(COUNTER_TIMEOUT_MILI_SECS, counter_b_timeout, counter, NULL);
}

static void counter_b_timeout(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->count_a++;
  counter->count_b++;
  LOG_DEBUG("Counter A: %d\n", counter->count_a);
  LOG_DEBUG("Counter B: %d\n", counter->count_b);

  soft_timer_start_millis(COUNTER_TIMEOUT_MILI_SECS, counter_a_timeout, counter, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counter = {
    .count_a = 0,
    .count_b = 0,
  };
  soft_timer_start_millis(COUNTER_TIMEOUT_MILI_SECS, counter_a_timeout, &counter, NULL);

  while (true) {
    wait();
  }

  return 0;
}
