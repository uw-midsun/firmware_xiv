// Firmware 102 Homework
// Vaaranan Yogalingam
// 2021-05-29

#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
#define PERIOD 500

typedef struct counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void increment_callback(SoftTimerId timer_id, void *context) {
  Counters *counter_data = context;
  counter_data->counter_a += 1;
  LOG_DEBUG("Counter A: %d\n", counter_data->counter_a);

  if (counter_data->counter_a % 2 == 0 && counter_data->counter_a != 0) {
    counter_data->counter_b += 1;
    LOG_DEBUG("Counter B: %d\n", counter_data->counter_b);
  }
  soft_timer_start_millis(PERIOD, increment_callback, context, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0, 0 };
  soft_timer_start_millis(PERIOD, increment_callback, &counters, NULL);

  while (true) {
    wait();
  }

  return 0;
}
