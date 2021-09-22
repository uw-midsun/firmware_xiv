#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define INCREMENT_HALF_SECOND 0.5

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_increment(SoftTimerId timer_id, void *context) {
  Counters *counters = context;
  counters->counter_a++;
  LOG_DEBUG("COUNTER A: %d\n", counters->counter_a);

  // Increments B every second (every two half seconds).
  if (counters->counter_a % 2 == 0) {
    counters->counter_b++;
    LOG_DEBUG("COUNTER B: %d\n", counters->counter_b);
  }

  soft_timer_start_seconds(INCREMENT_HALF_SECOND, prv_increment, counters, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counters = { .counter_a = 0, .counter_b = 0 };

  soft_timer_start_seconds(INCREMENT_HALF_SECOND, prv_increment, &counters, NULL);

  while (true) {
  }

  return 0;
}
