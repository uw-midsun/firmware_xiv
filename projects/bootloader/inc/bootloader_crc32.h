// Calculates the crc32 code for the flash memory for bootloader's
// jump to application functionality
#pragma once

#include <stddef.h>
#include <stdint.h>

// BOOTLOADER_APPLICATION_SIZE / 2048 = 54
#define crc32_code_buffer_size 54

// calculates the crc32 for the full address, with 2048 byte increments
// at a time, returning the final crc32 code
uint32_t calculated_application_crc32(uintptr_t address, size_t size);