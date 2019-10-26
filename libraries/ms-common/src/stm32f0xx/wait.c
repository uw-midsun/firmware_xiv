#include "wait.h"

#include "stm32f0xx.h"

void wait(void) {
  __WFI();
}
