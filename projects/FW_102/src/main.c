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

static void inc_counter_a(SoftTimerId timer_id, void *context);
static void inc_counter_b(SoftTimerId timer_id, void *context);

int main(void) {
  // Initialize soft timer, and soft timer dependency (interrupt is needed)
  interrupt_init();
  soft_timer_init();

  Counters loop_counter = {
    .counter_a = 0,
    .counter_b = 0,
  };

  // Increment counter a every 0.5s, counter b ever 1s.
  soft_timer_start_millis(500, inc_counter_a, &loop_counter, 0);
  soft_timer_start_millis(1000, inc_counter_b, &loop_counter, 0);

  // Wait for an interrupt
  while (true) {
    wait();
  }

  return 0;
}

// Increment a and print on LOG_DEBUG
static void inc_counter_a(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->counter_a += 1;
  LOG_DEBUG("Counter A: %d\n", counter->counter_a);

  soft_timer_start_millis(500, inc_counter_a, context, 0);
}

// Increment b and print on LOG_DEBUG
static void inc_counter_b(SoftTimerId timer_id, void *context) {
  Counters *counter = context;
  counter->counter_b += 1;
  LOG_DEBUG("Counter B: %d\n", counter->counter_b);

  soft_timer_start_millis(1000, inc_counter_b, context, 0);
}
