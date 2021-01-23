#include "jump_to_application.h"

#include <stdint.h>
#include <stdnoreturn.h>

#include "bootloader_mcu.h"

static noreturn __attribute__((naked)) void prv_perform_jump(uintptr_t pc, uintptr_t sp) {
  __asm(
      "msr msp, %[sp] \n"  // reset the stack pointer (msp) to sp
      "bx %[pc] \n"        // jump to pc
      // this bizarre syntax associates the "sp" and "pc" in asm with the pc and sp parameters
      // see http://www.ethernut.de/en/documents/arm-inline-asm.html
      :
      : [sp] "r"(sp), [pc] "r"(pc));
}

noreturn void jump_to_application(void) {
  // TODO(SOFT-413): check that this is safe with a crc of the application code
  // TODO(SOFT-413): deinitialize any libraries the bootloader uses before doing this
  prv_perform_jump(BOOTLOADER_APPLICATION_BASE_ADDR, BOOTLOADER_RAM_START);
}
