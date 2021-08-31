#include "bootloader_crc32.h"

#include "bootloader_mcu.h"
#include "config.h"
#include "crc32.h"
#include "flash.h"
#include "interrupt.h"
#include "log.h"
#include "persist.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

uint32_t s_crc32_codes_size = 54;
void setup_test(void) {
  flash_init();
  crc32_init();

}
void teardown_test(void) {}

void test_crc32_single_code(){
  uint32_t crc32_codes[s_crc32_codes_size];
  bootloader_crc32((uintptr_t)BOOTLOADER_APPLICATION_START, BOOTLOADER_APPLICATION_SIZE, (uint32_t *)&crc32_codes);
  for(int i=0; i < 54; i++)
    printf("Code %d is %d\n", i, crc32_codes[i]);

}

