#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"
#include "gpio.h"

#define SECOND_MS 1000
#define HALF_SECOND_MS 500

typedef struct Counters {
  uint16_t counter_a;
  uint16_t counter_b;
  
} Counters;

static void prv_increment(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;
  LOG_DEBUG("%d\n", storage->counter_a);
  if(storage->counter_a % 2 == 0){
    storage->counter_b++;
    LOG_DEBUG("%d\n", storage->counter_b);
  }
  soft_timer_start_millis(HALF_SECOND_MS, prv_increment, storage, NULL);
}
int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  Counters counter = { 0 };
  soft_timer_start_millis(HALF_SECOND_MS, prv_increment, &counter, NULL);
  while (true) {
    wait();
  }
  return 0;
}
