#include <stdint.h>  // for integer types

#include "interrupt.h"  // interrupts are required for soft timers
#include "log.h"
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "wait.h"        // for wait function

#define COUNTER_A_TIMER 500
#define COUNTER_B_TIMER 1000

typedef struct Counter {
  uint8_t counter_a;
  uint8_t counter_b;
} Counter;

void counter_a_callback(SoftTimerId timer_id, void *context) {
  Counter *mainCounter = context;
  mainCounter->counter_a++;

  LOG_DEBUG("Counter A: %d\n", mainCounter->counter_a);

  soft_timer_start_millis(COUNTER_A_TIMER, counter_a_callback, mainCounter, NULL);
}

void counter_b_callback(SoftTimerId timer_id, void *context) {
  Counter *mainCounter = context;
  mainCounter->counter_b++;

  LOG_DEBUG("Counter B: %d\n", mainCounter->counter_b);

  soft_timer_start_millis(COUNTER_B_TIMER, counter_b_callback, mainCounter, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  Counter mainCounter = { .counter_a = 0, .counter_b = 0 };

  soft_timer_start_millis(COUNTER_A_TIMER, counter_a_callback, &mainCounter, NULL);

  soft_timer_start_millis(COUNTER_B_TIMER, counter_b_callback, &mainCounter, NULL);

  while (true) {
    wait();
  }

  return 0;
}
