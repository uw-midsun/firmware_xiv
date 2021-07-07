#include <stdint.h>  // for integer types

#include "interrupt.h"  // interrupts are required for soft timers
#include "log.h"
#include "soft_timer.h"  // Software timers for scheduling
#include "status.h"
#include "test_helpers.h"
#include "wait.h"  // for wait function

// this test uses a single soft timer to increment counter_a every half a second, and counter_b
// every second.

typedef struct Counters {
  uint16_t counter;
} Counters;

#define INCREMENT_PERIOD_COUNTER_A_SEC (500)   // .5 of a second
#define INCREMENT_PERIOD_COUNTER_B_SEC (1000)  // 1 second

void prv_timer_callback_a(SoftTimerId timer_id, void *context) {
  Counters *data = context;  // cast void* to our struct so we can use it
  data->counter++;
  // log output
  LOG_DEBUG("Counter A: %d\n", data->counter);

  // start the timer again, so it keeps periodically flipping coins
  soft_timer_start_millis(INCREMENT_PERIOD_COUNTER_A_SEC, prv_timer_callback_a, data, NULL);
}

void prv_timer_callback_b(SoftTimerId timer_id, void *context) {
  Counters *data = context;  // cast void* to our struct so we can use it
  data->counter++;
  // log output
  LOG_DEBUG("Counter B: %d\n", data->counter);

  // start the timer again, so it keeps periodically flipping coins
  soft_timer_start_millis(INCREMENT_PERIOD_COUNTER_B_SEC, prv_timer_callback_b, data, NULL);
}

int main(void) {
  LOG_DEBUG("Initialize Soft Timer Counters\n");
  interrupt_init();         // interrupts must be initialized for soft timers to work
  soft_timer_init();        // soft timers must be initialized before using them
  Counters data_a = { 0 };  // we use this to initialize a struct to be all 0
  Counters data_b = { 0 };  // we use this to initialize a struct to be all 0

  soft_timer_start_millis(INCREMENT_PERIOD_COUNTER_A_SEC,  // timer duration
                          prv_timer_callback_a,            // function to call after timer
                          &data_a,
                          NULL);  // timer id - not needed here

  soft_timer_start_millis(INCREMENT_PERIOD_COUNTER_B_SEC,  // timer duration
                          prv_timer_callback_b,            // function to call after timer
                          &data_b,
                          NULL);  // timer id - not needed here

  while (true) {
    wait();  // waits until an interrupt is triggered rather than endlessly spinning
  }
  LOG_DEBUG(" End Soft Timer Counters\n");
  return 0;
}
