#include "delay.h"
#include <stdbool.h>
#include "stm32f0xx.h"

// needs soft_timer to be initalized

void delay_us(uint32_t t) {
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
