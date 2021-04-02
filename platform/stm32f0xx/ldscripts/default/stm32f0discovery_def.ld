/*
Linker subscript for STM32F072 definitions with 128K Flash and 16K RAM
*/

/* Memory Spaces Definitions */

ENTRY(Reset_Handler)

/* TODO: explicitly set aside the last page for calib */

MEMORY
{
  FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 128K
  RAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 16K
}

/* highest address of the user mode stack */
_estack = 0x20004000;
_flash_start = ORIGIN(FLASH);
_flash_size = LENGTH(FLASH);
_flash_page_size = 0x800;
