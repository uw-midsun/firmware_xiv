// Calculates the crc32 code for the flash memory for bootloader's
// jump to application functionality
#pragma once

#include <stddef.h>
#include <stdint.h>

// calculates the crc32 for the full address, with 2048 byte increments
// at a time, returning the final crc32 code
uint32_t calculated_application_crc32(uintptr_t address, size_t size);
