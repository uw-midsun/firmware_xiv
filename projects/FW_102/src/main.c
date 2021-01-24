#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void inc_counter(SoftTimerId timer_id, void *context);

int main(void) {
  // Initialize soft timer, and soft timer dependency (interrupt is needed)
  interrupt_init();
  soft_timer_init();

  Counters loop_counter = {
    .counter_a = 0,
    .counter_b = 0,
  };

  // Increment counter a every 0.5s, counter b ever 1s.
  soft_timer_start_millis(500, inc_counter, &loop_counter, 0);

  // Wait for an interrupt
  while (true) {
    wait();
  }

  return 0;
}

// Increment a and print on LOG_DEBUG
static void inc_counter(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->counter_a++;
  LOG_DEBUG("Counter A: %d\n", counter->counter_a);

  // Increment counter b every other time counter a is incremented
  if (counter->counter_a % 2 == 0) {
    counter->counter_b++;
    LOG_DEBUG("Counter B: %d\n", counter->counter_b);
  }

  soft_timer_start_millis(500, inc_counter, context, 0);
}
