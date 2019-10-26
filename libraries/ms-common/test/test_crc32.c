#include <inttypes.h>
#include "crc32.h"
#include "log.h"
#include "unity.h"

void setup_test(void) {
  crc32_init();
}

void teardown_test(void) {}

void test_crc32_basic(void) {
  uint8_t data[] = { 0x00 };
  uint32_t crc = crc32_arr(data, SIZEOF_ARRAY(data));
  LOG_DEBUG("0x%" PRIx32 "\n", crc);
  TEST_ASSERT_EQUAL_HEX(0xD202EF8D, crc);
}

void test_crc32_u32(void) {
  uint8_t data[] = { 0x12, 0x34, 0x56, 0x78 };
  uint32_t crc = crc32_arr(data, SIZEOF_ARRAY(data));
  LOG_DEBUG("0x%" PRIx32 "\n", crc);
  TEST_ASSERT_EQUAL_HEX(0x4A090E98, crc);
}

void test_crc32_bytes(void) {
  uint8_t data[] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
  uint32_t crc = crc32_arr(data, SIZEOF_ARRAY(data));
  LOG_DEBUG("0x%" PRIx32 "\n", crc);
  TEST_ASSERT_EQUAL_HEX(0xCBF43926, crc);
}
