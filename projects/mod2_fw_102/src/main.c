// Library inclusions:
#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

//Variable Definitions:
#define COUNTER_A_PERIOD_MS 500
uint8_t internal_counter = 1;

// Counters Struct
typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

// Counter functions:
void counter_func(SoftTimerId timer_id, void *context) {
  Counters *storage = context;

  LOG_DEBUG("Counter A: %i\n", storage->counter_a++);

  if (internal_counter % 2 == 0) { //Only increment counter_b every other 0.5s cycle
     LOG_DEBUG("Counter B: %i\n", storage->counter_b++);
  }

  internal_counter++; 
  soft_timer_start_millis(COUNTER_A_PERIOD_MS, counter_func, storage, NULL);
}

// Main Loop:
int main(void) {
  // Library initializations:
  interrupt_init();
  soft_timer_init();

  // Context initialization:
  Counters storage = { 0 };

  // Initialize timer loop:
  soft_timer_start_millis(2*COUNTER_A_PERIOD_MS,
                   counter_func,  // function callback
                   &storage, NULL);

  while (true) {
    wait();  // let functions run themselves indefinetly
  }
  return 0;
}
