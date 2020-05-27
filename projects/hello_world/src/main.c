#include "delay.h"  // For real-time delays
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "wait.h"

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
  bool half_second;
} Counters;

void callback(SoftTimerId timer_id, void *context) {
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

  soft_timer_start_millis(500, callback, counters, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counters;
  counters.counter_a = 0;
  counters.counter_b = 0;
  counters.half_second = false;

  soft_timer_start_millis(500, callback, &counters, NULL);

  while (true) {
    wait();
  }
  return 0;
}