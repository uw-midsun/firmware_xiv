/*
Linker subscript for memory definitions for the bootloader on STM32F072 with 128K Flash and 16K RAM.
See https://uwmidsun.atlassian.net/l/c/mHxmqjvn for more information.
*/

ENTRY(Reset_Handler)

/* bootloader size - this must be as small as possible while aligned on 2K and large enough */
_bootloader_size = 32K;

_flash_start = 0x08000000;
_flash_size = 128K;
_flash_page_size = 2K;

_ram_start = 0x20000000;
_ram_size = 16K;

_bootloader_start = _flash_start;
_config_page1_start = _bootloader_start + _bootloader_size;
_config_page2_start = _config_page1_start + _flash_page_size;
_application_start = _config_page2_start + _flash_page_size;

_application_size = _flash_size - (_application_start - _flash_start);

_vector_table_size = 0xc4; /* used for relocating the vector table to RAM for the application */

/* TODO: explicitly set aside the last page for calib */

MEMORY
{
  BOOTLOADER  (rx) : ORIGIN = _flash_start, LENGTH = _bootloader_size
  CONFIG_PAGE1 (r) : ORIGIN = _config_page1_start, LENGTH = _flash_page_size
  CONFIG_PAGE2 (r) : ORIGIN = _config_page2_start, LENGTH = _flash_page_size
  APPLICATION (rx) : ORIGIN = _application_start, LENGTH = _application_size
  RAM        (rwx) : ORIGIN = _ram_start, LENGTH = _ram_size
}

/* highest address of the user mode stack */
_estack = _ram_start + _ram_size;
