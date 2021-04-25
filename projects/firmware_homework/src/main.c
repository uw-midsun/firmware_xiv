// Standard library includes
#include <stdint.h>
#include <stdio.h>
// Midnight Sun includes
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
// Macros
#define DELAY_TIME 500

// Structure
typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

// Private function
static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *storage = context;

  storage->counter_a++;
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);

  storage->counter_b++;
  LOG_DEBUG("Counter B: %i\n", storage->counter_b);

  // Start the timer again
  soft_timer_start_millis(DELAY_TIME, prv_timer_callback, storage, NULL);
}

int main(void) {
  // Initializations
  interrupt_init();
  soft_timer_init();

  Counters storage = { 0 };

  // Timer functionalities
  soft_timer_start_millis(DELAY_TIME, prv_timer_callback, &storage, NULL);

  while (true) {
    wait();
  }

  return 0;
}
