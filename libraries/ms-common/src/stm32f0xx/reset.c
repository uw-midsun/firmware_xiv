#include "reset.h"
#include "core_cm0.h"

// Resets stm32f0xx board
void reset(void) {
  NVIC_SystemReset();
}
