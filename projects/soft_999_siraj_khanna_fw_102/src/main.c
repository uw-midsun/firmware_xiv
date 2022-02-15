#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define WAIT_MS 500

typedef struct count {
  uint8_t counter_a;
  uint8_t counter_b;
} count;

static void prv_timer_callback(SoftTimerId id, void *context) {
  count *c = context;

  if (c->counter_a % 2 == 0) {
    c->counter_a++;
    LOG_DEBUG("Counter A: %d \n", c->counter_a);
  } else {
    c->counter_a++;
    c->counter_b++;

    LOG_DEBUG("Counter A: %d \n", c->counter_a);
    LOG_DEBUG("Counter B: %d \n", c->counter_b);
  }

  // delay now
  soft_timer_start_millis(WAIT_MS, prv_timer_callback, c, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  struct count counter = { 0 };
  soft_timer_start_millis(WAIT_MS, prv_timer_callback, &counter, NULL);

  while (true) {
    wait();
  }
  return 0;
}
