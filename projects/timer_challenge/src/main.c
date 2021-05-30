#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
#define DELAY_TIME_MS 500

typedef struct counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_increment_callback(SoftTimerId timer_id, void *context) {
  Counters *counter_data = context;
  counter_data->counter_a++;
  LOG_DEBUG("Counter A: %d\n", counter_data->counter_a);

  if (counter_data->counter_a % 2 == 0) {
    counter_data->counter_b++;
    LOG_DEBUG("Counter B: %d\n", counter_data->counter_b);
  }
  soft_timer_start_millis(DELAY_TIME_MS, prv_increment_callback, counter_data, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counters = { 0 };
  soft_timer_start_millis(DELAY_TIME_MS, prv_increment_callback, &counters, NULL);

  while (true) {
    wait();
  }

  return 0;
}
