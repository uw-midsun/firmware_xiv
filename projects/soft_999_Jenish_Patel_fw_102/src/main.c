#include "interrupt.h"   // interrupts are required for soft timers
#include "log.h"         // for printing
#include "soft_timer.h"  // for soft timers
#include "wait.h"        // for wait function

#include <stdint.h>  // for integer types
#include <stdlib.h>  // for random numbers

#define COIN_FLIP_PERIOD_MS 1000  // milliseconds between coin flips

typedef struct CoinFlipStorage {
  uint16_t num_heads;
  uint16_t num_tails;
} CoinFlipStorage;

void prv_timer_callback(SoftTimerId timer_id, void *context) {
  CoinFlipStorage *storage = context;  // cast void* to our struct so we can use it
  uint8_t coinflip = rand() % 2;
  if (coinflip == 1)
    storage->num_heads++;
  else if (coinflip == 0)
    storage->num_tails++;

  // log output
  LOG_DEBUG("Num heads: %i, num tails: %i\n", storage->num_heads, storage->num_tails);

  // start the timer again, so it keeps periodically flipping coins
  soft_timer_start_millis(COIN_FLIP_PERIOD_MS, prv_timer_callback, &storage, NULL);
}

int main() {
  srand(14);  // seed the random number generator with (MS)XIV, our car number

  interrupt_init();   // interrupts must be initialized for soft timers to work
  soft_timer_init();  // soft timers must be initialized before using them

  CoinFlipStorage storage = { 0 };  // we use this to initialize a struct to be all 0

  soft_timer_start_millis(COIN_FLIP_PERIOD_MS,  // timer duration
                          prv_timer_callback,   // function to call after timer
                          &storage,             // automatically gets cast to void*
                          NULL);                // timer id - not needed here

  while (true) {
    wait();  // waits until an interrupt is triggered rather than endlessly spinning
  }

  return 0;
}
