/*
This is the linker script for the bootloaded application code itself on STM32F072.
*/

/* include the common memory spaces definitions subscript */
INCLUDE "../memory_spaces.ld"

/* include the application-specific sections management subscript */
INCLUDE "sections_flash.ld"
