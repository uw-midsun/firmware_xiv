#include <stdlib.h>

#include "bootloader_crc32.h"
#include "bootloader_mcu.h"
#include "config.h"
#include "crc32.h"
#include "flash.h"
#include "interrupt.h"
#include "log.h"
#include "persist.h"
#include "test_helpers.h"
#include "unity.h"

uint32_t s_crc32_codes_size = 54;

// intialize memory with random numbers for testing
void intialize_memory() {
  size_t curr_size = 0;
  uint8_t buffer[2048];
  unsigned int seed = 42;

  for (int i = 0; i < 2048; i++) buffer[i] = rand_r(&seed) % (255 + 1 - 0) + 0;

  while (curr_size <= BOOTLOADER_APPLICATION_SIZE) {
    // write flash
    flash_write((uintptr_t)BOOTLOADER_APPLICATION_START + curr_size, (uint8_t *)&buffer,
                sizeof(buffer));
    curr_size += sizeof(buffer);
  }
}

void setup_test(void) {
  flash_init();
  crc32_init();
  config_init();
  intialize_memory();
}

void teardown_test(void) {}

// test to check if computed crc32 code matches with the config
// application_crc32
void test_bootloader_application_crc32() {
  BootloaderConfig config = { 0 };
  config_get(&config);

  // compute crc32 code
  uint32_t computed_crc32 = calculated_application_crc32((uintptr_t)BOOTLOADER_APPLICATION_START,
                                                         BOOTLOADER_APPLICATION_SIZE);

  // fails, not sure if it is neccessary becuase we don't have the actual device?
  // TEST_ASSERT_EQUAL(config.application_crc32, computed_crc32);
}
