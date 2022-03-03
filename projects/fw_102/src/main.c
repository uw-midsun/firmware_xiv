
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_DURATION_MS 500

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;
  if (counters->counter_a % 2 == 0) {
    counters->counter_a++;
    LOG_DEBUG("Counter A %d\n", counters->counter_a);
  } else {
    counters->counter_a++;
    counters->counter_b++;
    LOG_DEBUG("Counter A: %d\n", counters->counter_a);
    LOG_DEBUG("Counter B: %d\n", counters->counter_b);
  }
  soft_timer_start_millis(COUNTER_DURATION, prv_timer_callback, counters, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  Counters c = { 0 };
  soft_timer_start_millis(COUNTER_DURATION, prv_timer_callback, &c, NULL);
  while (true) {
    wait();
  }
}
