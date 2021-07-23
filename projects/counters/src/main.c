#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define TIMER_INTERVAL_MS 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_counter_callback(SoftTimerId soft_timer_id, void *context) {
  Counters *counter_ptr = context;

  if (counter_ptr->counter_a % 2 == 0 && counter_ptr->counter_a != 0) {
    counter_ptr->counter_b++;
    LOG_DEBUG("Counter B: %d\n", counter_ptr->counter_b);
  }

  counter_ptr->counter_a++;
  LOG_DEBUG("Counter A: %d\n", counter_ptr->counter_a);

  soft_timer_start_millis(TIMER_INTERVAL_MS, prv_counter_callback, counter_ptr, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counter = { 0 };
  soft_timer_start_millis(TIMER_INTERVAL_MS, prv_counter_callback, &counter, NULL);

  while (true) {
    wait();
  }
  return 0;
}
