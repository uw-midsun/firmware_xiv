#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "delay.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// Counters structure with two counters
typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

// function to increment and print counter a 
void a_plus_one(SoftTimerId timer_id, void *context) {
  Counters *obj = context;
  obj->counter_a++;
  LOG_DEBUG("A = %d\n", obj->counter_a);
  soft_timer_start_millis(500, a_plus_one, obj, NULL);
}

// function to increment and print counter b
void b_plus_one(SoftTimerId timer_id, void *context) {
  Counters *obj = context;
  obj->counter_b++;
  LOG_DEBUG("B = %d\n", obj->counter_b);
  soft_timer_start_millis(1000, b_plus_one, obj, NULL);
}

// main function
int main(void) {
  
  Counters obj = { 0 };
  
  interrupt_init();
  soft_timer_init();

  soft_timer_start_millis(500, a_plus_one, &obj, NULL);
  soft_timer_start_millis(1000, b_plus_one, &obj, NULL);

  while (1) {
    wait();
  }
  
  return 0;
}
