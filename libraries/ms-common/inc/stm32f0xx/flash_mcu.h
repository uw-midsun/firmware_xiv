#pragma once

extern uint32_t _flash_start;
extern uint32_t _flash_size;
extern uint32_t _flash_page_size;

#define FLASH_BASE_ADDR ((uintptr_t)&_flash_start)
#define FLASH_SIZE_BYTES ((size_t)&_flash_size)
#define FLASH_END_ADDR (FLASH_BASE_ADDR + FLASH_SIZE_BYTES)

#define FLASH_MCU_WRITE_BYTES 4
#define FLASH_MCU_PAGE_BYTES ((size_t)&_flash_page_size)

typedef enum {
  FLASH_PAGE_0 = 0,     //
  NUM_FLASH_PAGES = 64  //
} FlashPage;
