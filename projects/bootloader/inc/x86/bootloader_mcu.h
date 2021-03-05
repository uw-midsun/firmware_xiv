#pragma once

#include "flash.h"
#include "flash_mcu.h"

// Below are the addresses of various sections in the x86 flash to emulate stm32

#define BOOTLOADER_CONFIG_PAGE_1_START ((void *)(FLASH_BASE_ADDR + 8 * FLASH_PAGE_BYTES))
#define BOOTLOADER_CONFIG_PAGE_2_START ((void *)(FLASH_BASE_ADDR + 9 * FLASH_PAGE_BYTES))
#define BOOTLOADER_APPLICATION_START ((void *)(FLASH_BASE_ADDR + 10 * FLASH_PAGE_BYTES))
#define BOOTLOADER_APPLICATION_SIZE (((size_t)0x1b000))
#define BOOTLOADER_VECTOR_TABLE_SIZE ((size_t)0xc4)

// BOOTLOADER_RAM_START and BOOTLOADER_RAM_SIZE are stm32 exclusive
