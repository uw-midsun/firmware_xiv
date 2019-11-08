#include "can_unpack_impl.h"

#include <stdint.h>
#include <string.h>

#include "can_msg.h"
#include "misc.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_UNPACK_IMPL_DLC 8

static const CanMessage s_msg = {
  .data_u8 = { 32, 64, 29, 76, 56, 21, 3, 1 },  //
  .dlc = TEST_CAN_UNPACK_IMPL_DLC,              //
};

void setup_test(void) {}

void teardown_test(void) {}

void test_can_unpack_impl_u8(void) {
  uint8_t f[8] = { 0 };
  TEST_ASSERT_OK(can_unpack_impl_u8(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, &f[0], &f[1], &f[2], &f[3],
                                    &f[4], &f[5], &f[6], &f[7]));
  TEST_ASSERT_EQUAL_UINT8_ARRAY(s_msg.data_u8, f, SIZEOF_ARRAY(s_msg.data_u8));
  TEST_ASSERT_OK(can_unpack_impl_u8(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, NULL, NULL, NULL, NULL, NULL,
                                    NULL, NULL,
                                    NULL));  // Ensure it doesn't segfault.

  memset(&f, 0, sizeof(f));
  TEST_ASSERT_OK(can_unpack_impl_u8(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, &f[0], NULL, &f[2], NULL,
                                    NULL, &f[5], NULL, NULL));
  TEST_ASSERT_EQUAL_UINT8(s_msg.data_u8[0], f[0]);
  TEST_ASSERT_EQUAL_UINT8(s_msg.data_u8[5], f[5]);
  TEST_ASSERT_EQUAL_UINT8(s_msg.data_u8[2], f[2]);

  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR,
                    can_unpack_impl_u8(&s_msg, TEST_CAN_UNPACK_IMPL_DLC - 1, NULL, NULL, NULL, NULL,
                                       NULL, NULL, NULL, NULL));
}

void test_can_unpack_impl_u16(void) {
  uint16_t f[4] = { 0 };
  TEST_ASSERT_OK(can_unpack_impl_u16(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, &f[0], &f[1], &f[2], &f[3]));
  TEST_ASSERT_EQUAL_UINT16_ARRAY(s_msg.data_u16, f, SIZEOF_ARRAY(s_msg.data_u16));
  TEST_ASSERT_OK(can_unpack_impl_u16(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, NULL, NULL, NULL,
                                     NULL));  // Ensure it doesn't segfault.

  memset(&f, 0, sizeof(f));
  TEST_ASSERT_OK(can_unpack_impl_u16(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, &f[0], NULL, &f[2], NULL));
  TEST_ASSERT_EQUAL_UINT8(s_msg.data_u16[0], f[0]);
  TEST_ASSERT_EQUAL_UINT8(s_msg.data_u16[2], f[2]);

  TEST_ASSERT_EQUAL(
      STATUS_CODE_INTERNAL_ERROR,
      can_unpack_impl_u16(&s_msg, TEST_CAN_UNPACK_IMPL_DLC - 1, NULL, NULL, NULL, NULL));
}

void test_can_unpack_impl_u32(void) {
  uint32_t f[2] = { 0 };
  TEST_ASSERT_OK(can_unpack_impl_u32(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, &f[0], &f[1]));
  TEST_ASSERT_EQUAL_UINT32_ARRAY(s_msg.data_u32, f, SIZEOF_ARRAY(s_msg.data_u32));
  TEST_ASSERT_OK(can_unpack_impl_u32(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, NULL,
                                     NULL));  // Ensure it doesn't segfault.

  memset(&f, 0, sizeof(f));
  TEST_ASSERT_OK(can_unpack_impl_u32(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, &f[0], NULL));
  TEST_ASSERT_EQUAL_UINT8(s_msg.data_u16[0], f[0]);

  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR,
                    can_unpack_impl_u32(&s_msg, TEST_CAN_UNPACK_IMPL_DLC - 1, NULL, NULL));
}

void test_can_unpack_impl_u64(void) {
  uint64_t f = 0;
  TEST_ASSERT_OK(can_unpack_impl_u64(&s_msg, TEST_CAN_UNPACK_IMPL_DLC, &f));
  TEST_ASSERT_EQUAL_UINT64(s_msg.data, f);
  TEST_ASSERT_OK(can_unpack_impl_u64(&s_msg, TEST_CAN_UNPACK_IMPL_DLC,
                                     NULL));  // Ensure it doesn't segfault.

  TEST_ASSERT_EQUAL(STATUS_CODE_INTERNAL_ERROR,
                    can_unpack_impl_u64(&s_msg, TEST_CAN_UNPACK_IMPL_DLC - 1, NULL));
}
