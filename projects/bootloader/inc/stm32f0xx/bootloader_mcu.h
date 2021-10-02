#pragma once

// Definitions for the bootloader.

#include <stdint.h>

// TODO(SOFT-416): Move this to ms-bootloader.

// The addresses of these values are defined in the bootloader linker script (memory_spaces.ld).
// If there's an undefined reference linker error from here, you're using the wrong linker script!
extern uint32_t _config_page1_start;
extern uint32_t _config_page2_start;
extern uint32_t _application_start;
extern uint32_t _application_size;
extern uint32_t _ram_start;
extern uint32_t _ram_size;
extern uint32_t _vector_table_size;
extern uint32_t _bootloader_start;
extern uint32_t _bootloader_size;

#define BOOTLOADER_CONFIG_PAGE_1_START ((void *)&_config_page1_start)
#define BOOTLOADER_CONFIG_PAGE_2_START ((void *)&_config_page2_start)
#define BOOTLOADER_APPLICATION_START ((void *)&_application_start)
#define BOOTLOADER_APPLICATION_SIZE ((size_t)&_application_size)
#define BOOTLOADER_RAM_START ((void *)&_ram_start)
#define BOOTLOADER_RAM_SIZE ((size_t)&_ram_size)
#define BOOTLOADER_VECTOR_TABLE_SIZE ((size_t)&_vector_table_size)
#define BOOTLOADER_DEFAULT_LOCATION ((size_t)&_bootloader_start)
#define BOOTLOADER_SIZE ((size_t)&_bootloader_size)
