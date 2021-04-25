// Standard library includes
#include <stdint.h>
#include <stdio.h>
// Midnight Sun includes
#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
// Macros
#define DELAY_TIME_MS 1000

// Structure
typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

// Private function
static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  // Counter A needs to be updated twice
  storage->counter_a++;
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);
  storage->counter_a++;
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);
  // Counter B needs to be updated once
  storage->counter_b++;
  LOG_DEBUG("Counter B: %i\n", storage->counter_b);
  // Start the timer again
  soft_timer_start_millis(DELAY_TIME_MS, prv_timer_callback, storage, NULL);
}

int main(void) {
  // Initializations
  interrupt_init();
  soft_timer_init();
  Counters storage = { 0 };
  // Timer call
  soft_timer_start_millis(DELAY_TIME_MS, prv_timer_callback, &storage, NULL);
  // Keep the program running
  while (true) {
    wait();
  }
  return 0;
}
