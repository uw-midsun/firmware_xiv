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

void test_crc32_combine(void) {
  uint8_t data[] = { 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39 };
  uint32_t crc = crc32_append_arr(data, 5, 0);
  crc = crc32_append_arr(data + 5, SIZEOF_ARRAY(data) - 5, crc);

  LOG_DEBUG("0x%" PRIx32 "\n", crc);
  TEST_ASSERT_EQUAL_HEX(0xCBF43926, crc);
}

void test_crc32_combine_large(void) {
  uint8_t data[2048];
  uint32_t crc = 0;

  uint8_t byte_val = 0;
  for (uint16_t i = 0; i < 54; ++i) {
    // set up the 2048 length data
    for (uint16_t i = 0; i < SIZEOF_ARRAY(data); ++i) {
      data[i] = byte_val;
      byte_val = (byte_val + 1) % 231;
    }

    crc = crc32_append_arr(data, SIZEOF_ARRAY(data), crc);
  }

  TEST_ASSERT_EQUAL_HEX(0xCB89AEA7, crc);
}
