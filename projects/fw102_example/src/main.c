//
// Created by Matthew Keller on 2022-01-22.
//
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"       // For real-time delays.
#include "interrupt.h"   // For interrupting stuff
#include "log.h"         // For printing stuff.
#include "soft_timer.h"  // Software timers for scheduling future events.
#include "wait.h"  //

#include "gpio.h"  // General Purpose I/O control.
#include "misc.h"  // Various helper functions/macros.

#define COUNTER_LENGTH 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

void increment_a(SoftTimerId timer_id, void *context) {
  Counters *counting = context;
  counting->counter_a++;
  printf("Counter A: %d\n", counting->counter_a);

  if( counting->counter_a % 2 == 0 ) {
      counting->counter_b ++;
      printf("Counter B: %d\n", counting->counter_b);
  }

  soft_timer_start_millis(COUNTER_LENGTH, increment_a, context, NULL);
}

int main(void) {
  interrupt_init();
  soft_timer_init();

  Counters counting = { 0 };
  //printf( "starting main\n" );
  soft_timer_start_millis(COUNTER_LENGTH, increment_a, &counting, NULL);

  while (true) {
    wait();
  }

  return 0;
}

