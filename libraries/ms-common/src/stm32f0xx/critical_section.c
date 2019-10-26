#include "critical_section.h"

#include <stdbool.h>

#include "stm32f0xx_misc.h"

bool critical_section_start(void) {
  if (!__get_PRIMASK()) {
    __disable_irq();
    // Interrupts got disabled.
    return true;
  }
  // Interrupts did not get disabled.
  return false;
}

void critical_section_end(bool disabled_in_scope) {
  if (__get_PRIMASK() && disabled_in_scope) {
    __enable_irq();
  }
}

void _critical_section_cleanup(bool *disabled_in_scope) {
  critical_section_end(*disabled_in_scope);
}
