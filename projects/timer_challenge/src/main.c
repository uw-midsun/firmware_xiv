#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

#include "main.h"  // for random numbers

#define A_PERIOD 500  // milliseconds between coin flips

typedef struct Counters {
  uint16_t counter_a;
  uint16_t counter_b;
} Counters;

void prv_timer_callback_A(SoftTimerId timer_id, void *context) {
  Counters *storage = context;  // cast void* to our struct so we can use it
  storage->counter_a++;

  soft_timer_start_millis(A_PERIOD, prv_timer_callback_A, storage, NULL);
  // log output
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);

  if (storage->counter_a % 2 == 0) {
    storage->counter_b++;
    LOG_DEBUG("COUNTER B: %i\n", storage->counter_b);
  }
  // start the timer again, so it keeps periodically flipping coins
}

int main() {
  interrupt_init();   // interrupts must be initialized for soft timers to work
  soft_timer_init();  // soft timers must be initialized before using them

  Counters storage = { 0 };  // we use this to initialize a struct to be all 0

  soft_timer_start_millis(A_PERIOD,              // timer duration
                          prv_timer_callback_A,  // function to call after timer
                          &storage,              // automatically gets cast to void*
                          NULL);                 // timer id - not needed here

  while (true) {
    wait();  // waits until an interrupt is triggered rather than endlessly spinning
  }
  return 0;
}
