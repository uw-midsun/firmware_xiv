#include "generic_can_msg.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "can.h"
#include "can_pack.h"
#include "can_unpack.h"
#include "log.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

void setup_test(void) {}

void teardown_test(void) {}

void test_can_message_convert(void) {
  CanMessage base_msg = { 0 };
  CAN_PACK_OVUV_DCDC_AUX(&base_msg, true, false, true, false);

  GenericCanMsg generic_msg = { 0 };
  TEST_ASSERT_OK(can_message_to_generic_can_message(&base_msg, &generic_msg));

  CanMessage out_msg = { 0 };
  TEST_ASSERT_OK(generic_can_msg_to_can_message(&generic_msg, &out_msg));
  TEST_ASSERT_EQUAL(0, memcmp(&base_msg, &out_msg, sizeof(CanMessage)));
}

void test_generic_msg_convert(void) {
  GenericCanMsg base_msg = {
    .id = 1 << 10,  // Must be < 11 bits to fit in a CanMessage
    .data = 0xABCDEF0123456789U,
    .dlc = 8U,
    .extended = false,  // Must be false for conversion to a CanMessage
  };
  for (uint16_t i = 0; i < (1 << 11); i++) {
    base_msg.id = i;

    CanMessage can_msg = { 0 };
    TEST_ASSERT_OK(generic_can_msg_to_can_message(&base_msg, &can_msg));

    GenericCanMsg out_msg = { 0 };
    TEST_ASSERT_OK(can_message_to_generic_can_message(&can_msg, &out_msg));
    TEST_ASSERT_EQUAL(base_msg.id, out_msg.id);
    TEST_ASSERT_EQUAL(base_msg.data, out_msg.data);
    TEST_ASSERT_EQUAL(base_msg.dlc, out_msg.dlc);
    TEST_ASSERT_EQUAL(base_msg.extended, out_msg.extended);
  }
}
