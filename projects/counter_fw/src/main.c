#include "interrupt.h"
#include "soft_timer.h"
#include "log.h"
#include "wait.h"

#include <stdint.h>

typedef struct Counters{

  uint8_t counter_a;
  uint8_t counter_b;

} Counters;

void counter_increment(SoftTimerId timer_id, void *context){
  Counters *count_inst = context;
  (count_inst->counter_a)++;
  LOG_DEBUG("Counter A: %d\n", count_inst->counter_a);
  if (((count_inst -> counter_a)%2)==0){
    (count_inst->counter_b)++;
    LOG_DEBUG("Counter B: %d\n", count_inst->counter_b);
  }
  soft_timer_start_millis(500, counter_increment, count_inst, NULL);
}

int main(void){
  interrupt_init();
  soft_timer_init();
  Counters count_inst = {0};

  soft_timer_start_millis(500, counter_increment, &count_inst, NULL);

  while(true){
    wait();
  }

  return 0;
}
