#include "interrupt.h"

#include "stm32f0xx_interrupt.h"

void interrupt_init(void) {
  stm32f0xx_interrupt_init();
}
