#include "jump_to_application.h"

#include <stdint.h>
#include <stdnoreturn.h>

#include "bootloader_mcu.h"
#include "stm32f0xx_misc.h"

static noreturn __attribute__((naked)) void prv_perform_jump(uint32_t sp, uint32_t pc) {
  __asm(
      "msr msp, %[sp] \n"  // reset the stack pointer (msp) to sp
      "bx %[pc] \n"        // jump to pc
      // this bizarre syntax associates the "sp" and "pc" in asm with the sp and pc parameters
      // see http://www.ethernut.de/en/documents/arm-inline-asm.html
      :
      : [sp] "r"(sp), [pc] "r"(pc));
}

noreturn void jump_to_application(void) {
  // TODO(SOFT-413): check that this is safe with a crc of the application code
  // TODO(SOFT-413): deinitialize any libraries the bootloader uses before doing this

  // we don't want any interrupts while we're messing with the vector table
  __disable_irq();
  // use MSP (main stack pointer) as the stack pointer
  __set_CONTROL(0);
  // fuck there's no VTOR on STM32F072
  // work around it with http://kevincuzner.com/2018/11/13/bootloader-for-arm-cortex-m0-no-vtor/
  //SCB->VTOR = BOOTLOADER_APPLICATION_BASE_ADDR;

  // the initial stack pointer and reset handler pointer are the first two 32-bit words in the image
  uint32_t *application_flash = BOOTLOADER_APPLICATION_BASE_ADDR;
  uint32_t initial_sp = application_flash[0];
  uint32_t reset_handler_pc = application_flash[1];

  prv_perform_jump(initial_sp, reset_handler_pc);
}
