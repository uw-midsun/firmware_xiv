#include "reset.h"
#include "stm32f0xx.h"

// Resets stm32f0xx board
void reset(void) {
  NVIC_SystemReset();
}
