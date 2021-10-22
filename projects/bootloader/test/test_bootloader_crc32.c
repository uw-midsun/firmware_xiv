#include <stdlib.h>

#include "bootloader_crc32.h"

#include "bootloader_mcu.h"
#include "crc32.h"
#include "flash.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_datagram.h"
#include "persist.h"
#include "test_helpers.h"
#include "unity.h"

// intialize memory with random numbers for testing
void initialize_memory(void) {
  size_t curr_size = 0;
  // writing the memory using a buffer with 2048 bytes at a time
  uint8_t buffer[2048];

  // generating random values in range of uint8(0->255)
  for (uint16_t i = 0; i < 2048; i++) {
    buffer[i] = i % 231;
  }

  while (curr_size < BOOTLOADER_APPLICATION_SIZE) {
    // write flash
    flash_write((uintptr_t)BOOTLOADER_APPLICATION_START + curr_size, buffer, sizeof(buffer));
    curr_size += sizeof(buffer);
  }
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  gpio_init();
  soft_timer_init();
  flash_init();
  crc32_init();
  initialize_memory();
}

void teardown_test(void) {}

// test to check if computed crc32 code matches with the config
// application_crc32
void test_bootloader_application_crc32() {
  // compute crc32 code
  uint32_t computed_crc32 = calculate_application_crc32();
  TEST_ASSERT_EQUAL(3900764041, computed_crc32);
}
