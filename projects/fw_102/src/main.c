#include <stdbool.h>
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define ITERATION_TIMEOUT 0.5

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

// Timeout callback
static void timeout_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;

  counters->counter_a++;
  LOG_DEBUG("Counter A: %i\n", counters->counter_a);

  if (counters->counter_a % 2 == 0) {
    counters->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  // Schedule another timer - this creates a periodic timer
  soft_timer_start_seconds(ITERATION_TIMEOUT, timeout_callback, counters, NULL);
}

int main(void) {
  // Init modules
  gpio_init();
  interrupt_init();
  soft_timer_init();

  struct Counters counters;

  // Begin a timer
  soft_timer_start_seconds(ITERATION_TIMEOUT, timeout_callback, &counters, NULL);

  // Infinite loop
  while (true) {
    // Wait for interrupts
    wait();
  }

  return 0;
}
