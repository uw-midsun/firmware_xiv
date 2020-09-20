#include <stdint.h>      // for integer types

#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for logging
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "wait.h"        // for wait function

#define COUNTER_DELAY_MS 500

typedef struct Counter {
  uint8_t counters[2];
} Counter;

typedef struct TimerData {
  Counter main_counter;
  bool incrementBoth;
} TimerData;

static void prv_increment_and_print_counter(int counter_num, Counter *main_counter) {
  main_counter->counters[counter_num]++;
  LOG_DEBUG("Counter %c: %d\n", counter_num+65, main_counter->counters[counter_num]);
}

static void prv_counter_callback(SoftTimerId timer_id, void *context) {
  TimerData *timer_data = context;

  prv_increment_and_print_counter(0, &timer_data->main_counter);
  if (timer_data->incrementBoth) {
    prv_increment_and_print_counter(1, &timer_data->main_counter);
  }
  timer_data->incrementBoth = !timer_data->incrementBoth;

  soft_timer_start_millis(COUNTER_DELAY_MS, prv_counter_callback, timer_data, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  Counter main_counter = { .counters = { 0 } };
  TimerData timer_data = { .main_counter = main_counter, .incrementBoth = false};

  soft_timer_start_millis(COUNTER_DELAY_MS, prv_counter_callback, &timer_data, NULL);

  while (true) {
    wait();
  }

  return 0;
}
