#include <stdlib.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
#define COUNTER_INCREMENT_MS 500  // 500 milliseconds between incrementing counter A

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *count = context;
  count->counter_a += 1;
  LOG_DEBUG("Counter A:%d\n", count->counter_a);

  if (count->counter_a % 2 == 0) {  // this is so counter b is incremented every other function
                                    // call, so it is incremented every 1000 ms
    count->counter_b += 1;
    LOG_DEBUG("Counter B:%d\n", count->counter_b);
  }

  soft_timer_start_millis(COUNTER_INCREMENT_MS, prv_timer_callback, count, NULL);
}

int main() {
  interrupt_init();
  soft_timer_init();

  Counters count = { 0 };

  soft_timer_start_millis(COUNTER_INCREMENT_MS, prv_timer_callback, &count, NULL);

  while (true) {
    wait();
  }
}
