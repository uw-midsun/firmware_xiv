#pragma once
// Raw Flash memory API
// Warning: Offers no protection against overwriting program code
//
// Note that due to the way flash works, writes can only flip bits from 1 -> 0
// and must be aligned In order to switch bits from 0 -> 1, the entire page must
// be erased
#include <stddef.h>
#include <stdint.h>

#include "flash_mcu.h"
#include "status.h"

#define FLASH_ADDR_TO_PAGE(addr) \
  (((uintptr_t)(addr) - (uintptr_t)FLASH_BASE_ADDR) / FLASH_PAGE_BYTES)
#define FLASH_PAGE_TO_ADDR(page) ((uintptr_t)(page)*FLASH_PAGE_BYTES + (uintptr_t)FLASH_BASE_ADDR)
#define FLASH_WRITE_BYTES FLASH_MCU_WRITE_BYTES
#define FLASH_PAGE_BYTES FLASH_MCU_PAGE_BYTES

StatusCode flash_init(void);

StatusCode flash_read(uintptr_t address, size_t read_bytes, uint8_t *buffer, size_t buffer_len);

// Note that this API does not support overwriting flash - once written, data
// must be cleared by erasing the entire page before rewriting.
//
// address must be aligned to FLASH_WRITE_BYTES and buffer_len must be a
// multiple of FLASH_WRITE_BYTES
StatusCode flash_write(uintptr_t address, uint8_t *buffer, size_t buffer_len);

// Flips the entire page to 1's
StatusCode flash_erase(FlashPage page);
