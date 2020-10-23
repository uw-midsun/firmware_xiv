#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

#define COUNTER_MS_A 500   // milliseconds between coin flips
#define COUNTER_MS_B 1000  // milliseconds between coin flips

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

static void prv_blink_timeout_a(SoftTimerId timer_id, void *context) {
  Counters *storage = context;  // cast void* to our struct so we can use it
  storage->counter_a++;
  // log output
  LOG_DEBUG("Counter A:  %i\n", storage->counter_a);
  // Schedule another timer - this creates a periodic timer
  soft_timer_start_millis(COUNTER_MS_A, prv_blink_timeout_a, storage, NULL);
}

static void prv_blink_timeout_b(SoftTimerId timer_id, void *context) {
  Counters *storage = context;  // cast void* to our struct so we can use it
  storage->counter_b++;
  // log output
  LOG_DEBUG("Counter B:  %i\n", storage->counter_b);
  // Schedule another timer - this creates a periodic timer
  soft_timer_start_millis(COUNTER_MS_B, prv_blink_timeout_b, storage, NULL);
}

int main(void) {
  LOG_DEBUG("Hello World!\n");

  interrupt_init();
  soft_timer_init();

  Counters storage = { 0 };  // we use this to initialize a struct to be all 0

  soft_timer_start_millis(COUNTER_MS_A, prv_blink_timeout_a, &storage, NULL);
  soft_timer_start_millis(COUNTER_MS_B, prv_blink_timeout_b, &storage, NULL);
  // Infinite loop
  while (true) {
    // Wait for interrupts
    wait();
  }
  return 0;
}
\n
