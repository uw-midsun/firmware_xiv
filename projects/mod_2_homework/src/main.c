#include <stdint.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

typedef struct {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void timer_callback(SoftTimerId timer_id, void *ctx) {
  Counters *ctr = ctx;
  ctr->counter_a++;
  LOG_DEBUG("Counter A: %i\n", ctr->counter_a);
  if (ctr->counter_a % 2 == 0) {
    ctr->counter_b++;
    LOG_DEBUG("Counter B: %i\n", ctr->counter_b);
  }
  soft_timer_start_seconds(0.5, timer_callback, ctr, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters ctr = { 0 };

  soft_timer_start_seconds(0.5, timer_callback, &ctr, NULL);

  while (true) {
    wait();
  }

  return 0;
}
