#include "bcd.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}

// Test if the bcd and its respective dec value are equal
void test_conversion(void) {
  uint8_t bcd = 0b10011001;
  uint8_t dec = 99;
  TEST_ASSERT_EQUAL(bcd, dec_to_bcd(dec));
  TEST_ASSERT_EQUAL(dec, bcd_to_dec(bcd));
}

void test_invalid_dec_to_bcd(void) {
  uint8_t dec = 101;
  TEST_ASSERT_EQUAL(INVALID, dec_to_bcd(dec));
}

void test_invalid_bcd_to_dec(void) {
  uint8_t bcd = 0b11111111;
  TEST_ASSERT_EQUAL(INVALID, bcd_to_dec(bcd));
}
