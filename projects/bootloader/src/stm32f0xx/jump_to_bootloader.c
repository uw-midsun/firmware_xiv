#include "jump_to_bootloader.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

#include "bootloader_mcu.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_syscfg.h"

static noreturn __attribute__((naked)) void prv_perform_jump(uint32_t sp, uint32_t pc) {
  __asm(
      "msr msp, %[sp] \n"  // reset the main stack pointer (msp) to sp
      "bx %[pc] \n"        // jump to pc

      // this bizarre syntax associates the "sp" and "pc" in asm with the sp and pc parameters
      // see http://www.ethernut.de/en/documents/arm-inline-asm.html
      :
      : [sp] "r"(sp), [pc] "r"(pc));
}

void jump_to_bootloader(void) {
  // Disable all interrupts so their is no interference when working with vector tables
  __disable_irq();

  // Reset the vector table to bootloader's vector table
  SYSCFG_MemoryRemapConfig(
      SYSCFG_MemoryRemap_SRAM);  // change where the vector table is stored to beginning of flash

  // story memory location of bootloader starting point at the beginning of flash?
  uint32_t *bootloader_in_flash = BOOTLOADER_DEFAULT_LOCATION;
  // setting the stack pointer to the bootloader location now
  uint32_t initial_sp = bootloader_in_flash[0];
  uint32_t reset_handler_pc = bootloader_in_flash[1];

  // re-enable interrupts
  __enable_irq();
  // jump to bootloader
  prv_perform_jump(initial_sp, reset_handler_pc);
}
