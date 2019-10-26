#include "flash.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_FLASH_PAGE (NUM_FLASH_PAGES - 1)
#define TEST_FLASH_ADDR FLASH_PAGE_TO_ADDR(TEST_FLASH_PAGE)

void setup_test(void) {
  flash_init();
  flash_erase(TEST_FLASH_PAGE);
}

void teardown_test(void) {
  // Be sure to erase the flash just in case an application uses it
  flash_erase(TEST_FLASH_PAGE);
}

void test_flash_basic_rw(void) {
  uint8_t data[] = { 0x12, 0x34, 0x56, 0x78 };
  uint8_t read[SIZEOF_ARRAY(data)];

  StatusCode ret = flash_write(TEST_FLASH_ADDR, data, SIZEOF_ARRAY(data));
  TEST_ASSERT_OK(ret);

  ret = flash_read(TEST_FLASH_ADDR, SIZEOF_ARRAY(data), read, SIZEOF_ARRAY(read));
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, read, SIZEOF_ARRAY(data));
}

void test_flash_overwrite(void) {
  uint8_t data[] = { 0x55, 0x55, 0x55, 0x55 };
  uint8_t read[SIZEOF_ARRAY(data)];

  StatusCode ret = flash_write(TEST_FLASH_ADDR, data, SIZEOF_ARRAY(data));
  TEST_ASSERT_OK(ret);

  // Overwrite with the same data
  // STM32 does not allow overwriting at all
  ret = flash_write(TEST_FLASH_ADDR, data, SIZEOF_ARRAY(data));
  TEST_ASSERT_NOT_OK(ret);

  // Try modifying some bits from 1 -> 0
  data[0] = 0x11;
  data[2] = 0x11;
  ret = flash_write(TEST_FLASH_ADDR, data, SIZEOF_ARRAY(data));
  TEST_ASSERT_NOT_OK(ret);

  // Try modifying some bits from 0 -> 1
  data[1] = 0xff;
  ret = flash_write(TEST_FLASH_ADDR, data, SIZEOF_ARRAY(data));
  TEST_ASSERT_NOT_OK(ret);
}

void test_flash_misaligned_write(void) {
  uint32_t data = 0x00;

  StatusCode ret = flash_write(TEST_FLASH_ADDR + 1, (uint8_t *)&data, sizeof(data));
  TEST_ASSERT_NOT_OK(ret);

  ret = flash_write(TEST_FLASH_ADDR, (uint8_t *)&data, FLASH_WRITE_BYTES + 1);
  TEST_ASSERT_NOT_OK(ret);
}

void test_flash_out_of_bounds(void) {
  uint32_t data = 0x00;

  StatusCode ret = flash_write(0, (uint8_t *)&data, sizeof(data));
  TEST_ASSERT_NOT_OK(ret);

  ret = flash_write(FLASH_END_ADDR + FLASH_WRITE_BYTES, (uint8_t *)&data, sizeof(data));
  TEST_ASSERT_NOT_OK(ret);

  ret = flash_erase(NUM_FLASH_PAGES);
  TEST_ASSERT_NOT_OK(ret);
}

void test_flash_verify_num_pages(void) {
  TEST_ASSERT_EQUAL(FLASH_SIZE_BYTES / FLASH_PAGE_BYTES, NUM_FLASH_PAGES);
}
