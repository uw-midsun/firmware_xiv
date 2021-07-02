#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void soft_counter_callback(const SoftTimerId timer_id, void *context) {
  Counters *counter1 = (Counters *)context;
  counter1->counter_a = counter1->counter_a + 1;
  LOG_DEBUG("counter a: %i \n", counter1->counter_a);
  if (counter1->counter_a % 2 == 0) {
    counter1->counter_b = counter1->counter_b + 1;
    LOG_DEBUG("counter b: %i \n", counter1->counter_b);
  }
  soft_timer_start_millis(500, soft_counter_callback, counter1, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  Counters counter1 = { .counter_b = 0, .counter_a = 0 };
  soft_timer_start_millis(500, soft_counter_callback, &counter1, NULL);
  while (true) {
  }
  return 0;
}