#include <stdint.h>  // for integet types
#include <stdlib.h>
#include "interrupt.h"   // interrupting soft timers
#include "log.h"         // log debugging
#include "soft_timer.h"  // timer initial
#include "wait.h"        // using wait function

// counter miliseconds defined
#define counter_time_MS 500

typedef struct Counters {
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

// callback for the counter A
static void counting_callback(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  storage->counter_a++;

  // using the multiple to print counter b every second
  if (storage->counter_a % 2 == 0) {
    storage->counter_b++;
    LOG_DEBUG("Counter B: %i\n", storage->counter_b);
  }
  // print out every 500 ms
  LOG_DEBUG("Counter A: %i\n", storage->counter_a);

  // START TIMER AGAIN THE NEXT
  soft_timer_start_millis(counter_time_MS, counting_callback, storage, NULL);
}

int main(void) {
  Counters storage = { 0 };
  interrupt_init();
  soft_timer_init();

  // parameters: counting time, callback for the timer, pass the pointer for the struct
  soft_timer_start_millis(counter_time_MS, counting_callback, &storage, NULL);

  while (true) {
    wait();  // cpu sleep
  }
  return 0;
}
