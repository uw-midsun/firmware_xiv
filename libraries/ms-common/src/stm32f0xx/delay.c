#include "delay.h"

#include <stdbool.h>
#include <stddef.h>

#include "soft_timer.h"
#include "stm32f0xx.h"
#include "stm32f0xx_misc.h"
#include "wait.h"

// needs soft_timer to be initalized

static void prv_delay_us_sys_clock(uint32_t t) {
  uint32_t current_time = TIM_GetCounter(TIM2);  // use TIM2
  uint32_t end_time = current_time + t;
  // since t is uint32, there is a max of 1 rollover
  bool rollover = (end_time < current_time);

  while (rollover || current_time < end_time) {
    // update time
    uint32_t time = TIM_GetCounter(TIM2);
    if (time < current_time) {  // rollover detection
      rollover = false;
    }
    current_time = time;
  }
}

static void prv_delay_it(SoftTimerId timer_id, void *context) {
  volatile bool *block = context;
  *block = false;
}

static void prv_delay_us_soft_timer(uint32_t t) {
  volatile bool block = true;
  soft_timer_start(t, prv_delay_it, (void *)&block, NULL);
  while (block) {
    wait();
  }
}

void delay_us(uint32_t t) {
  // use softtimer if interrupts are not disabled
  // use clock delay if iterrupts are disabled
  if (__get_PRIMASK()) {  // interrupts disabled
    prv_delay_us_sys_clock(t);
  } else {
    prv_delay_us_soft_timer(t);
  }
}
