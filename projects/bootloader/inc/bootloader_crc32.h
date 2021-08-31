#pragma once

#include <stddef.h>
#include <stdint.h>

// calculates the crc32 for the full address, with 2048 byte increments
// and inserts them in crc32_codes array
void bootloader_crc32(uintptr_t address, size_t size, uint32_t *crc32_codes);