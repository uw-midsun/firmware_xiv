//
// Created by Karim Alatrash on 2021-03-08.
//
#include <stdbool.h>     // for booleans
#include <stdint.h>      // for integer types
#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

typedef struct Counter {
  uint8_t counter_a;
  uint8_t counter_b;
} Counter;

void increment_counter(SoftTimerId timer_id, void *context) {
  Counter *count = context;

  if (count->counter_a == count->counter_b) {
    count->counter_a++;
    LOG_DEBUG("Counter A: %i\n", count->counter_a);
  } else {
    count->counter_b++;
    LOG_DEBUG("Counter B: %i\n", count->counter_b);
  }

  soft_timer_start_millis(500, increment_counter, count, NULL);
}

int main(void) {
  // LOG_DEBUG("Hello World!\n");

  interrupt_init();   // interrupts must be initialized for soft timers to work
  soft_timer_init();  // soft timers must be initialized before using them

  Counter count = { 0 };

  soft_timer_start_millis(500,                // timer duration
                          increment_counter,  // function to call after timer
                          &count,             // automatically gets cast to void*
                          NULL);              // timer id - not needed here

  while (true) {
    wait();  // waits until an interrupt is triggered rather than endlessly spinning
  }

  return 0;
}
