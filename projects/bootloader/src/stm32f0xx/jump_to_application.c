#include "jump_to_application.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

#include "bootloader_crc32.h"
#include "bootloader_mcu.h"
#include "config.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_syscfg.h"

static noreturn __attribute__((naked)) void prv_perform_jump(uint32_t sp, uint32_t pc) {
  __asm(
      "msr msp, %[sp] \n"  // reset the stack pointer (msp) to sp
      "bx %[pc] \n"        // jump to pc
      // this bizarre syntax associates the "sp" and "pc" in asm with the sp and pc parameters
      // see http://www.ethernut.de/en/documents/arm-inline-asm.html
      :
      : [sp] "r"(sp), [pc] "r"(pc));
}

void jump_to_application(void) {
  BootloaderConfig config = { 0 };
  config_get(&config);

  // get the computed crc32 code
  uint32_t computed_crc32 = calculated_application_crc32((uintptr_t)BOOTLOADER_APPLICATION_START,
                                                         BOOTLOADER_APPLICATION_SIZE);

  if (config.application_crc32 != computed_crc32) {
    LOG_DEBUG("CRC32 codes do not match returning");
    return;
  }

  __disable_irq();   // we don't want any interrupts while we're messing with the vector table
  __set_CONTROL(0);  // use MSP (main stack pointer) as the stack pointer

  // We need to point the system at the application's interrupt vector table.
  // Every ARM chip except the Cortex M0 (which the STM32F072 has) has a Vector Table Offset
  // Register (VTOR) which we could use to remap the vector table.
  // The STM32F072 doesn't have one, so instead we copy the vector table to SRAM and remap address 0
  // (the vector table's address) to SRAM rather than flash.
  // See the STM32F0xx manual, section 2.5, "Physical remap".
  memcpy(BOOTLOADER_RAM_START, BOOTLOADER_APPLICATION_START, BOOTLOADER_VECTOR_TABLE_SIZE);
  SYSCFG_MemoryRemapConfig(SYSCFG_MemoryRemap_SRAM);

  // the first two 32-bit words in the vector table are the initial SP and the reset handler address
  uint32_t *application_flash = BOOTLOADER_APPLICATION_START;
  uint32_t initial_sp = application_flash[0];
  uint32_t reset_handler_pc = application_flash[1];

  __enable_irq();
  prv_perform_jump(initial_sp, reset_handler_pc);
}
