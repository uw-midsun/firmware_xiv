#include "hal_test_helpers.h"

#include <stdint.h>

#include "stm32f0xx_tim.h"

void _test_soft_timer_set_counter(uint32_t counter_value) {
  TIM_SetCounter(TIM2, counter_value);
  return;
}
