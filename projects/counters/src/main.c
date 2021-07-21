#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_counter_callback(SoftTimerId soft_timer_id, void *context) {
  Counters *counter_ptr = (Counters *)context;
  uint8_t *counter_a_ptr = &(counter_ptr->counter_a);
  uint8_t *counter_b_ptr = &(counter_ptr->counter_b);

  if ((*counter_a_ptr == 0) || ((*counter_a_ptr) % 2 == 1)) {
    (*counter_a_ptr)++;
    LOG_DEBUG("Counter A: %d\n", *counter_a_ptr);

  } else {
    (*counter_a_ptr)++;
    (*counter_b_ptr)++;
    LOG_DEBUG("Counter B: %d\n", *counter_b_ptr);
    LOG_DEBUG("Counter A: %d\n", *counter_a_ptr);
  }

  soft_timer_start_millis(500, prv_counter_callback, counter_ptr, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters counter = { 0 };
  soft_timer_start_millis(500, prv_counter_callback, &counter, NULL);

  while (true) {
  }
  return 0;
}
