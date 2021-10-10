#include "jump.h"

static noreturn __attribute__((naked)) void prv_perform_jump(uint32_t sp, uint32_t pc) {
  __asm(
      "msr msp, %[sp] \n"  // reset the main stack pointer (msp) to sp
      "bx %[pc] \n"        // jump to pc

      // this bizarre syntax associates the "sp" and "pc" in asm with the sp and pc parameters
      // see http://www.ethernut.de/en/documents/arm-inline-asm.html
      :
      : [sp] "r"(sp), [pc] "r"(pc));
}
