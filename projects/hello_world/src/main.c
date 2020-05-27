#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "wait.h"        // for the wait function
#define HALF_SECOND_MS 500;

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
  bool half_second;
} Counters;

static void prev_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;

  if (counters->half_second == true) {
    counters->counter_a++;
    LOG_DEBUG("Counter A: %i\n", counters->counter_a);
  } else {
    counters->counter_a++;
    counters->counter_b++;
    LOG_DEBUG("Counter A: %i\n", counters->counter_a);
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  counters->half_second = !counters->half_second;

  soft_timer_start_millis(HALF_SECOND_MS, prev_timer_callback, counters, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };

  soft_timer_start_millis(HALF_SECOND_MS, prev_timer_callback, &counters, NULL);

  while (true) {
    wait();
  }
  return 0;
}
