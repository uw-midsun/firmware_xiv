// Library Initializations
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "delay.h"       // For real-time delays.
#include "interrupt.h"   // For interrupting stuff
#include "log.h"         // For printing stuff.
#include "soft_timer.h"  // Software timers for scheduling future events.

#include "gpio.h"  // General Purpose I/O control.
#include "misc.h"  // Various helper functions/macros.

typedef struct struct_counter {
  uint8_t counter_a;
  uint8_t counter_b;
} Counter;

void counter_timer_callback(SoftTimerId timer_id, void *context) {
  Counter *counter_storage = context;

  uint8_t value = counter_storage->counter_a % 2;

  counter_storage->counter_a++;
  LOG_DEBUG("Counter A: %u\n", counter_storage->counter_a);

  if (value == 0) {
    counter_storage->counter_b++;
    LOG_DEBUG("Counter B: %u\n", counter_storage->counter_b);
  }

  soft_timer_start_millis(500, counter_timer_callback, counter_storage, NULL);
}

int main(void) {
  // Stuff
  interrupt_init();
  soft_timer_init();

  Counter counter;

  counter.counter_a = 0;
  counter.counter_b = 0;

  soft_timer_start_millis(500, counter_timer_callback, &counter, NULL);

  while (true) {
  }

  return 0;
}
