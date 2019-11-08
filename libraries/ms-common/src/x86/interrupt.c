#include "interrupt.h"

#include "x86_interrupt.h"

void interrupt_init(void) {
  x86_interrupt_init();
}
