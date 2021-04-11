#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
};

struct Counters *counter_init() {
  struct Counters *out = malloc(sizeof(struct Counters));
  out->counter_a = 0;
  out->counter_b = 0;
  return out;
}

void delay(unsigned milliseconds) {
  clock_t pause;
  clock_t start;

  pause = milliseconds * (CLOCKS_PER_SEC / 1000);
  start = clock();
  while ((clock() - start) < pause) {
    continue;
  }
}

void increment(SoftTimerId timer_id, void *context) {
  struct Counters *temp = context;
  temp->counter_a += 1;
  LOG_DEBUG("Counter A: %d\n", temp->counter_a);
  delay(0.5);
  temp->counter_a += 1;
  LOG_DEBUG("Counter A: %d\n", temp->counter_a);

  temp->counter_b += 1;
  LOG_DEBUG("Counter B: %d\n", temp->counter_b);

  soft_timer_start_millis(500, increment, temp, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  struct Counters *temp = counter_init();

  soft_timer_start_millis(500, increment, temp, NULL);

  while (1) {
    wait();
  }

  return 0;
}
