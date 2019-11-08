#include "can_ack.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

typedef enum {
  TEST_CAN_ACK_DEVICE_A = 0,
  TEST_CAN_ACK_DEVICE_B,
  TEST_CAN_ACK_DEVICE_C,
  TEST_CAN_ACK_DEVICE_UNRECOGNIZED
} TestCanAckDevice;

static CanAckRequests s_ack_requests;

typedef struct TestResponse {
  CanMessageId msg_id;
  uint16_t device;
  CanAckStatus status;
  uint16_t num_remaining;
} TestResponse;

static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  LOG_DEBUG("ACK handled: status %d from %d (msg %d) (%d remaining)\n", status, device, msg_id,
            num_remaining);
  TestResponse *data = context;
  data->msg_id = msg_id;
  data->device = device;
  data->status = status;
  data->num_remaining = num_remaining;

  if (status == CAN_ACK_STATUS_UNKNOWN) {
    LOG_DEBUG("Returning unknown code\n");
    return status_code(STATUS_CODE_UNKNOWN);
  }

  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  can_ack_init(&s_ack_requests);
}

void teardown_test(void) {}

void test_can_ack_handle_devices(void) {
  TestResponse data = { 0 };
  CanMessage can_msg = {
    .source_id = TEST_CAN_ACK_DEVICE_A,  //
    .type = CAN_MSG_TYPE_ACK,            //
    .msg_id = 0x2,                       //
  };
  CanAckRequest ack_request = {
    .callback = prv_ack_callback,  //
    .context = &data,              //
  };

  ack_request.expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_CAN_ACK_DEVICE_A);
  can_ack_add_request(&s_ack_requests, 0x4, &ack_request);
  ack_request.expected_bitset =
      CAN_ACK_EXPECTED_DEVICES(TEST_CAN_ACK_DEVICE_A, TEST_CAN_ACK_DEVICE_B, TEST_CAN_ACK_DEVICE_C);
  can_ack_add_request(&s_ack_requests, 0x2, &ack_request);
  can_ack_add_request(&s_ack_requests, 0x6, &ack_request);
  ack_request.expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_CAN_ACK_DEVICE_A);
  can_ack_add_request(&s_ack_requests, 0x2, &ack_request);

  LOG_DEBUG("Handling ACK for ID %d, device %d\n", can_msg.msg_id, can_msg.source_id);
  StatusCode ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_OK(ret);

  // Expect to update the 1st 0x2 ACK request
  TEST_ASSERT_EQUAL(can_msg.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(2, data.num_remaining);

  LOG_DEBUG("Handling duplicate ACK\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_OK(ret);

  // Should've updated the 2nd 0x2 ACK request
  TEST_ASSERT_EQUAL(can_msg.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(0, data.num_remaining);

  // 3rd duplicate 0x2 ACK should fail
  LOG_DEBUG("Handling duplicate ACK 2\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_EQUAL(STATUS_CODE_UNKNOWN, ret);

  // Valid ACK (new device) - 0x2
  can_msg.source_id = TEST_CAN_ACK_DEVICE_B;
  LOG_DEBUG("Handling ACK for ID %d, device %d\n", can_msg.msg_id, can_msg.source_id);
  ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(can_msg.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(1, data.num_remaining);

  // Send invalid device ACK - should be able to send another ACK
  can_msg.source_id = TEST_CAN_ACK_DEVICE_C;
  can_msg.data = CAN_ACK_STATUS_UNKNOWN;
  LOG_DEBUG("Handling ACK from invalid device\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(0, data.num_remaining);

  // Unrecognized device ACK - should just be ignored
  can_msg.source_id = TEST_CAN_ACK_DEVICE_UNRECOGNIZED;
  can_msg.data = 0;
  LOG_DEBUG("Handling ACK from unrecognized device\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_NOT_OK(ret);

  // 0x2, valid ACK - should still have 1 remaining
  can_msg.source_id = TEST_CAN_ACK_DEVICE_C;
  LOG_DEBUG("Handling ACK from valid device\n");
  ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(0, data.num_remaining);

  // 2 ACK requests should be removed
  TEST_ASSERT_EQUAL(2, s_ack_requests.num_requests);
}

void test_can_ack_expiry(void) {
  // Basic expiry test
  volatile TestResponse data = { 0 };
  CanAckRequest ack_request = {
    .callback = prv_ack_callback,  //
    .context = &data,              //
    .expected_bitset = 0x1,        //
  };

  can_ack_add_request(&s_ack_requests, 0x2, &ack_request);

  while (data.msg_id == 0) {
  }

  TEST_ASSERT_EQUAL(0x2, data.msg_id);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_TIMEOUT, data.status);
  TEST_ASSERT_EQUAL(0, s_ack_requests.num_requests);
}

void test_can_ack_expiry_moved(void) {
  // Ensure that ACK expiry can handle being shuffled around
  volatile TestResponse data = { 0 };
  CanMessage can_msg = {
    .source_id = TEST_CAN_ACK_DEVICE_A,  //
    .type = CAN_MSG_TYPE_ACK,            //
    .msg_id = 0x4,                       //
  };
  CanAckRequest ack_request = {
    .callback = prv_ack_callback,                                        //
    .context = &data,                                                    //
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_CAN_ACK_DEVICE_A),  //
  };

  can_ack_add_request(&s_ack_requests, 0x4, &ack_request);
  can_ack_add_request(&s_ack_requests, 0x2, &ack_request);

  StatusCode ret = can_ack_handle_msg(&s_ack_requests, &can_msg);
  TEST_ASSERT_OK(ret);

  TEST_ASSERT_EQUAL(can_msg.msg_id, data.msg_id);
  TEST_ASSERT_EQUAL(1, s_ack_requests.num_requests);

  while (data.msg_id == can_msg.msg_id) {
  }

  TEST_ASSERT_EQUAL(0x2, data.msg_id);
  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_TIMEOUT, data.status);
  TEST_ASSERT_EQUAL(0, s_ack_requests.num_requests);
}
