#include <stddef.h>
#include <stdint.h>
#include "crc15.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {
  crc15_init_table();
}

void teardown_test(void) {}

void test_crc15_calculate_example(void) {
  // example from table 24
  uint8_t data[2] = { 0x00, 0x01 };
  uint16_t pec = 0x3D6E;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_rdcva(void) {
  // datasheet p.55
  uint8_t data[2] = { 0x00, 0x04 };
  uint16_t pec = 0x07C2;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_adcv(void) {
  // datasheet p.55
  uint8_t data[2] = { 0x03, 0x70 };
  uint16_t pec = 0xAF42;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_clrcell(void) {
  // datasheet p.55
  uint8_t data[] = { 0x07, 0x11 };
  uint16_t pec = 0xC9C0;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_pladc(void) {
  // datasheet p.55
  uint8_t data[] = { 0x9F, 0x14 };
  uint16_t pec = 0x1C48;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_wrcomm(void) {
  // datasheet p.56
  uint8_t data[] = { 0x07, 0x21 };
  uint16_t pec = 0x24B2;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

/* the datasheet may be wrong?
void test_crc15_calculate_wrcomm_i2c_slave_result() {
  // datasheet p.56
  uint8_t data[] = { 0x6A, 0x08, 0x00, 0x18, 0x0A, 0xA9 };
  TEST_ASSERT_EQUAL(0x6DFB, crc15_calculate(data, sizeof(data) / sizeof(data[0])));
}
*/

void test_crc15_calculate_wrcomm_slave_result(void) {
  // datasheet p.56
  uint8_t data[] = { 0x85, 0x50, 0x8A, 0xA0, 0x8C, 0xC9 };
  uint16_t pec = 0x89A4;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_stcomm(void) {
  // datasheet p.56
  uint8_t data[] = { 0x07, 0x23 };
  uint16_t pec = 0xB9E4;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_rdcomm(void) {
  // datasheet p.56
  uint8_t data[] = { 0x07, 0x22 };
  uint16_t pec = 0x32D6;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_rdcomm_i2c_data_result(void) {
  // datasheet p.56
  uint8_t data[] = { 0x6A, 0x07, 0x70, 0x17, 0x7A, 0xA1 };
  uint16_t pec = 0xD0DE;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_rdcomm_slave_result(void) {
  // datasheet p.57
  uint8_t data[] = { 0x75, 0x5F, 0x7A, 0xAF, 0x7C, 0xCF };
  uint16_t pec = 0xF2BA;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_readcfg(void) {
  uint8_t data[] = { 0x00, 0x02 };
  uint16_t pec = 0x2B0A;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}

void test_crc15_calculate_wrcfg(void) {
  uint8_t data[] = { 0x00, 0x01 };
  uint16_t pec = 0x3D6E;

  TEST_ASSERT_EQUAL(pec, crc15_calculate(data, SIZEOF_ARRAY(data)));
}
