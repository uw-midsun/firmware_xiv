#include <stdint.h>
#include <stdlib.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define COIN_FLIP_PERIOD_MS 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_increment(SoftTimerId timer_id, void *context) {
  Counters *temp = context;
  temp->counter_a++;
  LOG_DEBUG("Counter A: %d\n", temp->counter_a);
  delay_s(0.5);
  temp->counter_a++;
  LOG_DEBUG("Counter A: %d\n", temp->counter_a);

  temp->counter_b++;
  LOG_DEBUG("Counter B: %d\n", temp->counter_b);

  soft_timer_start_millis(COIN_FLIP_PERIOD_MS, prv_increment, temp, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters temp = { 0 };

  soft_timer_start_millis(COIN_FLIP_PERIOD_MS, prv_increment, &temp, NULL);

  while (1) {
    wait();
  }

  return 0;
}
