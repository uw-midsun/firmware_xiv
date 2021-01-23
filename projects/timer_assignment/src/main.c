#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define TIMER_PERIOD 500  // timer period of 0.5 seconds

typedef struct Counters {  // stores counter a and b counts
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counters = context;

  counters->counter_a++;  // increments a every cycle

  LOG_DEBUG("Counter A: %i\n", counters->counter_a);

  if (counters->counter_a % 2 == 0) {  // increments b if a is on an even cycle
    counters->counter_b++;
    LOG_DEBUG("Counter B: %i\n", counters->counter_b);
  }

  // starts timer again after callback function completes
  soft_timer_start_millis(TIMER_PERIOD, prv_timer_callback, counters, NULL);
}

int main(void) {
  LOG_DEBUG("Timer Homework Baybee!\n");

  interrupt_init();   // i dont understand how these inits work
  soft_timer_init();  // but imma do some research

  Counters counters = { 0 };  // initialize the struct to be all 0

  soft_timer_start_millis(TIMER_PERIOD,        // timer duration
                          prv_timer_callback,  // callback function
                          &counters,           // passes in initialized counter struct
                          NULL);               // no timer id baybee
  while (true) {
    wait();
  }

  return 0;
}

