#include "jump_to_bootloader.h"

#include <stdint.h>
#include <stdnoreturn.h>
#include <string.h>

#include "bootloader_mcu.h"
#include "jump.h"
#include "stm32f0xx_misc.h"
#include "stm32f0xx_syscfg.h"

void jump_to_bootloader(void) {
  // Disable all interrupts so their is no interference when working with vector tables
  __disable_irq();

  // Reset the vector table to bootloader's vector table
  SYSCFG_MemoryRemapConfig(
      SYSCFG_MemoryRemap_Flash);  // change where the vector table is stored to beginning of flash

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
