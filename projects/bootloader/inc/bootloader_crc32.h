#pragma once

#include <stddef.h>
#include <stdint.h>
uint32_t single_code_crc32();

void bootloader_crc32(uintptr_t address, size_t size, uint32_t *crc32_codes);