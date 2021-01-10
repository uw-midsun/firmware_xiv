//
// Created by henry on 2021-01-09.
//
#include <stdint.h>

#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;
void callback(SoftTimerId id, void *context) {
  static bool b_increment = false;
  Counters *Counter = context;
  LOG_DEBUG("%d\n", ++Counter->counter_a);
  if (b_increment) {
    LOG_DEBUG("%d\n", ++Counter->counter_b);
    b_increment = false;
  } else {
    b_increment = true;
  }
}
int main(void) {
  Counters Counter = { 0 };
  interrupt_init();
  soft_timer_init();
  while (true) {
    if(!soft_timer_inuse()){
      soft_timer_start(0.5 * 1000000, callback, &Counter, NULL);
    }
  }
  return 0;
}
