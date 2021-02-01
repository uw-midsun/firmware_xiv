#include <stdlib.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_inc_counter_a(SoftTimerId timer_id, void *context) {
  Counters *c1 = context;
  c1->counter_a++;

  LOG_DEBUG("Counter A: %d\n", c1->counter_a);

  if (c1->counter_a % 2 == 0) {
    c1->counter_b++;
    LOG_DEBUG("Counter B: %d\n", c1->counter_b);
  }

  soft_timer_start_millis(500, prv_inc_counter_a, c1, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters c1 = { 0 };

  soft_timer_start_millis(500, prv_inc_counter_a, &c1, NULL);
  while (true) {
  }
  return 0;
}
