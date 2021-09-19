//Library inclusions:
#include "log.h"
#include "interrupt.h" 
#include "soft_timer.h"
#include "wait.h"
#include <stdint.h> 

//Counters Struct
typedef struct Counters{
  uint8_t counter_a;
  uint8_t counter_b;
} Counters;

//Counter functions:
void counter_a_func(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  uint8_t counter_a = 1;
  
  LOG_DEBUG("Counter A: %i\n", storage->counter_a++);
  
  soft_timer_start(500000,
                   counter_a_func, 
                   storage, 
                   NULL);
}

void counter_b_func(SoftTimerId timer_id, void *context) {
  Counters *storage = context;
  uint8_t counter_b = 1;
  
  LOG_DEBUG("Counter B: %i\n", storage->counter_b++);
  
  soft_timer_start(1000000,
                   counter_b_func, 
                   storage, 
                   NULL);
}

//Main Loop:

int main (void) {
    //Library initializations:
    interrupt_init();
    soft_timer_init();

    //Context initialization:
    Counters storage = { 0 };
  

    //Initialize timer loop:
    soft_timer_start(10000, 
                    counter_a_func, //function callback
                    &storage, 
                    NULL);

     soft_timer_start(50000, 
                      counter_b_func, //functioncallback
                      &storage, 
                      NULL);

    while (true) {
        wait(); //let functions run themselves indefinetly
    }
    return 0;
}