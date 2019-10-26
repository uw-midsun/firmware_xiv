#pragma once

#define FLASH_BASE_ADDR 0x400
#define FLASH_SIZE_BYTES 0x10000
#define FLASH_END_ADDR (FLASH_BASE_ADDR + FLASH_SIZE_BYTES)

#define FLASH_MCU_WRITE_BYTES 4
#define FLASH_MCU_PAGE_BYTES 0x400

typedef enum {
  FLASH_PAGE_0 = 0,     //
  NUM_FLASH_PAGES = 64  //
} FlashPage;
