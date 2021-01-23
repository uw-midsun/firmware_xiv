/*
This is the linker script for the bootloader itself on STM32F072.
*/

/* include the common memory spaces definitions subscript */
INCLUDE "memory_spaces.ld"

/* include the bootloader-specific sections management subscript */
INCLUDE "sections_flash_bootloader.ld"
