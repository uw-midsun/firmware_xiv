#include "can_pack_impl.h"

#include <stdint.h>
#include <stdio.h>

#include "can_msg.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_PACK_ID 1
#define TEST_CAN_PACK_SOURCE_ID 1
#define TEST_CAN_PACK_TYPE CAN_MSG_TYPE_DATA

// Since all the methods use the same test framework just define it as a macro
// to reduce redundant code. It is a bit magic but it is relatively
// understandable if you realize macros are just replaced with what they
// represent at compile time.
#define TEST_CAN_PACK_IMPL_FRAMEWORK(pack_call)                  \
  do {                                                           \
    CanMessage msg = { 0 };                                      \
    test_can_pack_impl_data data = { .data_u64 = 1 };            \
    size_t size;                                                 \
    for (size = 0; size < CAN_PACK_IMPL_MAX_DLC; size++) {       \
      TEST_ASSERT_OK((pack_call));                               \
      TEST_ASSERT_EQUAL_UINT64(data.data_u64, msg.data);         \
      TEST_ASSERT_EQUAL(size, msg.dlc);                          \
      TEST_ASSERT_EQUAL(TEST_CAN_PACK_SOURCE_ID, msg.source_id); \
      TEST_ASSERT_EQUAL(TEST_CAN_PACK_ID, msg.msg_id);           \
      TEST_ASSERT_EQUAL(TEST_CAN_PACK_TYPE, msg.type);           \
      data.data_u64 <<= 8;                                       \
    }                                                            \
    ++size;                                                      \
    TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, (pack_call));    \
  } while (0)

typedef union test_can_pack_impl_data {
  uint64_t data_u64;
  uint32_t data_u32[2];
  uint16_t data_u16[4];
  uint8_t data_u8[8];
} test_can_pack_impl_data;

void setup_test(void) {}

void teardown_test(void) {}

void test_can_pack_impl_u8(void) {
  TEST_CAN_PACK_IMPL_FRAMEWORK(can_pack_impl_u8(&msg, TEST_CAN_PACK_SOURCE_ID, TEST_CAN_PACK_ID,
                                                size, data.data_u8[0], data.data_u8[1],
                                                data.data_u8[2], data.data_u8[3], data.data_u8[4],
                                                data.data_u8[5], data.data_u8[6], data.data_u8[7]));
}

void test_can_pack_impl_u16(void) {
  TEST_CAN_PACK_IMPL_FRAMEWORK(can_pack_impl_u16(&msg, TEST_CAN_PACK_SOURCE_ID, TEST_CAN_PACK_ID,
                                                 size, data.data_u16[0], data.data_u16[1],
                                                 data.data_u16[2], data.data_u16[3]));
}

void test_can_pack_impl_u32(void) {
  TEST_CAN_PACK_IMPL_FRAMEWORK(can_pack_impl_u32(&msg, TEST_CAN_PACK_SOURCE_ID, TEST_CAN_PACK_ID,
                                                 size, data.data_u32[0], data.data_u32[1]));
}

void test_can_pack_impl_u64(void) {
  TEST_CAN_PACK_IMPL_FRAMEWORK(
      can_pack_impl_u64(&msg, TEST_CAN_PACK_SOURCE_ID, TEST_CAN_PACK_ID, size, data.data_u64));
}
