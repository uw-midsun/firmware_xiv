#include "cobs.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_COBS_MAX_BUFFER_SIZE 300

static void prv_encode_decode(const uint8_t *data, size_t data_len) {
  uint8_t encoded_data[COBS_MAX_ENCODED_LEN(TEST_COBS_MAX_BUFFER_SIZE)];
  uint8_t decoded_data[COBS_MAX_ENCODED_LEN(TEST_COBS_MAX_BUFFER_SIZE)];

  size_t encoded_len = SIZEOF_ARRAY(encoded_data);
  TEST_ASSERT_OK(cobs_encode(data, data_len, encoded_data, &encoded_len));
  LOG_DEBUG("Encoded %zu bytes into %zu bytes\n", data_len, encoded_len);
  size_t decoded_len = SIZEOF_ARRAY(decoded_data);
  TEST_ASSERT_OK(cobs_decode(encoded_data, encoded_len, decoded_data, &decoded_len));
  LOG_DEBUG("Decoded %zu bytes into %zu bytes (original %zu bytes)\n", encoded_len, decoded_len,
            data_len);

  TEST_ASSERT_EQUAL(data_len, decoded_len);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(data, decoded_data, data_len);
}

void setup_test(void) {}
void teardown_test(void) {}

void test_cobs_all_zero(void) {
  uint8_t data[] = { 0, 0, 0, 0, 0 };
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}

void test_cobs_no_zero_short(void) {
  uint8_t data[] = { 1, 2, 3, 4, 5 };
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}

void test_cobs_trailing_zero(void) {
  uint8_t data[] = { 1, 2, 3, 4, 5, 0 };
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}

void test_cobs_leading_zero(void) {
  uint8_t data[] = { 0, 1, 2, 3, 4, 5 };
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}

void test_cobs_consecutive_zeros(void) {
  uint8_t data[] = { 0, 1, 2, 0, 0, 3, 0, 4, 5, 0 };
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}

void test_cobs_long_non_zero_no_code(void) {
  // Long run of non-zero bytes, but not long enough to hit the special case
  uint8_t data[253] = { 0 };
  for (size_t i = 0; i < SIZEOF_ARRAY(data); i++) {
    data[i] = i + 1;
  }
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}

void test_cobs_long_non_zero_code(void) {
  // Long run of non-zero bytes - hit the special case
  uint8_t data[254] = { 0 };
  for (size_t i = 0; i < SIZEOF_ARRAY(data); i++) {
    data[i] = i + 1;
  }
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}

void test_cobs_long_non_zero_code_more_data(void) {
  // Long run of non-zero bytes - hit the special case + more data
  uint8_t data[255] = { 0 };
  for (size_t i = 0; i < SIZEOF_ARRAY(data); i++) {
    data[i] = i + 1;
  }
  prv_encode_decode(data, SIZEOF_ARRAY(data));
}
