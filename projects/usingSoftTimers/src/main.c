
// Created by Sudhish M on 2020-09-25.
// using_soft_timers

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COUNTER_PERIOD_MS 500

typedef struct counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *counter_ptr = context;

  counter_ptr->counter_a++;
  LOG_DEBUG("Counter A: %i \n", counter_ptr->counter_a);

  if (counter_ptr->counter_a % 2 == 0) {
    counter_ptr->counter_b++;
    LOG_DEBUG("Counter B: %i \n", counter_ptr->counter_b);
  }

  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_timer_callback, counter_ptr, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counter_test = { 0 };

  soft_timer_start_millis(COUNTER_PERIOD_MS, prv_timer_callback, &counter_test, NULL);

  while (true) {
    wait();
  }

  return 0;
}
