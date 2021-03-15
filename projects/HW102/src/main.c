#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define SECOND 1000
#define HALF_SECOND 500

typedef struct Counters {
  uint16_t counter_a;
  uint16_t counter_b;
} Counters;

void increment_a(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;
  LOG_DEBUG("%d\n", storage->counter_a);
  soft_timer_start_millis(HALF_SECOND, increment_a, &storage, NULL);
}

void increment_b(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_b++;
  LOG_DEBUG("%d\n", storage->counter_b);
  soft_timer_start_millis(SECOND, increment_b, &storage, NULL);
}

int main() {
  Counters counter = { 0 };
  soft_timer_start_millis(HALF_SECOND, increment_a, &counter, NULL);
  soft_timer_start_millis(SECOND, increment_b, &counter, NULL);
  while (true) {
    wait();
  }
  return 0;
}
