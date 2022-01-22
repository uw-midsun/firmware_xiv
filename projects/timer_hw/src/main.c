#include "interrupt.h"
#include "soft_timer.h"
#include "log.h"
#include "wait.h"

#include <stdlib.h>
#include <stdint.h>

#define COUNTER_PERIOD_MS 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void prv_timer_callback_2(SoftTimerId timer_id, void *context);

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;
  
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);
  
  soft_timer_start_millis(COUNTER_PERIOD_MS,
                          prv_timer_callback_2, 
                          storage, 
                          NULL);
}

void prv_timer_callback_2(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;
  storage->counter_b++;

  LOG_DEBUG("Counter A: %i\n", storage->counter_a);
  LOG_DEBUG("Counter B: %i\n", storage->counter_b);

  soft_timer_start_millis(COUNTER_PERIOD_MS,
                          prv_timer_callback,
                          storage,
                          NULL);
}


int main() {
  interrupt_init();
  soft_timer_init();
  
  Counters storage = { 0 };
  
  soft_timer_start_millis(COUNTER_PERIOD_MS,
                          prv_timer_callback,
                          &storage,
                          NULL);
  
  while (true) {
    wait();
  }
  
  return 0;
}
