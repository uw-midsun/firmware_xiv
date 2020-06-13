#include <stdint.h>
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

#define HALF_SECOND_MS 500  //

typedef struct CounterStorage {
  uint16_t counter_a;
  uint16_t counter_b;
} CounterStorage;

void prv_timer_call(const SoftTimerId timer_id, void *context) {
  // cast void* to our struct so we can use it
  CounterStorage *storage = context;
  storage->counter_a++;
  storage->counter_b++;

  // Prints Every half-second
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);

  // Prints Every Second
  if (storage->counter_b % 2 == 0) {
    LOG_DEBUG("Counter B: %i\n", storage->counter_b / 2);
  }

  // start the timer again, so it keeps periodically flipping coins
  soft_timer_start_millis(HALF_SECOND, prv_timer_call, &storage, NULL);
}

int main() {
  interrupt_init();                // Initalize interrupt
  soft_timer_init();               // Initalize soft_timer
  CounterStorage storage = { 0 };  // Make the struct 0

  soft_timer_start_millis(HALF_SECOND, prv_timer_call, &storage, NULL);

  while (true) {
    wait();
  }

  return 0;
}
