#include "can.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_UNKNOWN_MSG_ID 0xA
#define TEST_CAN_DEVICE_ID 0x1

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  CanMessage *rx_msg = context;
  *rx_msg = *msg;

  if (msg->msg_id == TEST_CAN_UNKNOWN_MSG_ID) {
    *ack_reply = CAN_ACK_STATUS_UNKNOWN;
  }

  return STATUS_CODE_OK;
}

static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  uint16_t *device_acked = context;
  *device_acked = device;

  return STATUS_CODE_OK;
}

static StatusCode prv_ack_callback_status(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                          uint16_t num_remaining, void *context) {
  CanAckStatus *ret_status = context;
  *ret_status = status;

  return STATUS_CODE_OK;
}

static void prv_clock_tx(void) {
  Event e = { 0 };
  StatusCode ret = event_process(&e);
  TEST_ASSERT_OK(ret);
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_TX, e.id);

  bool processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);
}

void setup_test(void) {
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  TEST_ASSERT_OK(ret);
}

void teardown_test(void) {}

void test_can_basic(void) {
  volatile CanMessage rx_msg = { 0 };
  can_register_rx_handler(0x6, prv_rx_callback, &rx_msg);
  can_register_rx_handler(0x1, prv_rx_callback, &rx_msg);
  can_register_rx_handler(0x5, prv_rx_callback, &rx_msg);

  CanMessage msg = {
    .msg_id = 0x5,              //
    .type = CAN_MSG_TYPE_DATA,  //
    .data = 0x1,                //
    .dlc = 1,                   //
  };

  // Begin CAN transmit request
  StatusCode ret = can_transmit(&msg, NULL);
  TEST_ASSERT_OK(ret);
  prv_clock_tx();

  Event e = { 0 };
  // Wait for RX
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_RX, e.id);
  bool processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);

  TEST_ASSERT_EQUAL(msg.msg_id, rx_msg.msg_id);
  TEST_ASSERT_EQUAL(msg.data, rx_msg.data);
}

void test_can_filter(void) {
  volatile CanMessage rx_msg = { 0 };
  can_add_filter(0x2);

  can_register_rx_handler(0x1, prv_rx_callback, &rx_msg);
  can_register_rx_handler(0x2, prv_rx_callback, &rx_msg);

  CanMessage msg = {
    .msg_id = 0x1,               //
    .type = CAN_MSG_TYPE_DATA,   //
    .data = 0x1122334455667788,  //
    .dlc = 8,                    //
  };

  StatusCode ret = can_transmit(&msg, NULL);
  TEST_ASSERT_OK(ret);
  prv_clock_tx();

  msg.msg_id = 0x2;
  ret = can_transmit(&msg, NULL);
  TEST_ASSERT_OK(ret);
  prv_clock_tx();

  Event e = { 0 };
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_RX, e.id);
  TEST_ASSERT_EQUAL(1, e.data);

  bool processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);

  TEST_ASSERT_EQUAL(msg.msg_id, rx_msg.msg_id);
  TEST_ASSERT_EQUAL(msg.data, rx_msg.data);
}

void test_can_ack(void) {
  volatile CanMessage rx_msg = { 0 };
  volatile uint16_t device_acked = CAN_MSG_INVALID_DEVICE;

  CanMessage msg = {
    .msg_id = 0x1,               //
    .type = CAN_MSG_TYPE_DATA,   //
    .data = 0x1122334455667788,  //
    .dlc = 8,                    //
  };

  CanAckRequest ack_req = {
    .callback = prv_ack_callback,                                     //
    .context = &device_acked,                                         //
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_CAN_DEVICE_ID),  //
  };

  // Register handler so we ACK
  can_register_rx_handler(0x1, prv_rx_callback, &rx_msg);

  StatusCode ret = can_transmit(&msg, &ack_req);
  TEST_ASSERT_OK(ret);
  prv_clock_tx();

  Event e = { 0 };
  // Handle RX of message and attempt transmit of ACK
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_RX, e.id);
  bool processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);
  prv_clock_tx();

  // Handle RX of ACK
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_RX, e.id);
  processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);

  TEST_ASSERT_EQUAL(TEST_CAN_DEVICE_ID, device_acked);
}

void test_can_ack_expire(void) {
  volatile CanAckStatus ack_status = NUM_CAN_ACK_STATUSES;
  CanMessage msg = {
    .msg_id = 0x1,               //
    .type = CAN_MSG_TYPE_DATA,   //
    .data = 0x1122334455667788,  //
    .dlc = 8,                    //
  };

  CanAckRequest ack_req = {
    .callback = prv_ack_callback_status,                              //
    .context = &ack_status,                                           //
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_CAN_DEVICE_ID),  //
  };

  StatusCode ret = can_transmit(&msg, &ack_req);
  TEST_ASSERT_OK(ret);

  while (ack_status == NUM_CAN_ACK_STATUSES) {
  }

  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_TIMEOUT, ack_status);
}

void test_can_ack_status(void) {
  volatile CanAckStatus ack_status = NUM_CAN_ACK_STATUSES;
  volatile CanMessage rx_msg = { 0 };
  CanMessage msg = {
    .msg_id = TEST_CAN_UNKNOWN_MSG_ID,
    .type = CAN_MSG_TYPE_DATA,
    .data = 0x1122334455667788,
    .dlc = 8,
  };

  CanAckRequest ack_req = {
    .callback = prv_ack_callback_status,                              //
    .context = &ack_status,                                           //
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_CAN_DEVICE_ID),  //
  };

  can_register_rx_handler(TEST_CAN_UNKNOWN_MSG_ID, prv_rx_callback, &rx_msg);

  StatusCode ret = can_transmit(&msg, &ack_req);
  TEST_ASSERT_OK(ret);
  prv_clock_tx();

  Event e = { 0 };
  // Handle RX of message and attempt transmit of ACK
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_RX, e.id);
  bool processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);
  prv_clock_tx();

  // Handle RX of ACK
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_RX, e.id);
  processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);

  TEST_ASSERT_EQUAL(CAN_ACK_STATUS_UNKNOWN, ack_status);
}

void test_can_default(void) {
  volatile CanMessage rx_msg = { 0 };
  CanMessage msg = {
    .msg_id = 0x1,              //
    .type = CAN_MSG_TYPE_DATA,  //
    .data = 0x1,                //
    .dlc = 1,                   //
  };

  can_register_rx_default_handler(prv_rx_callback, &rx_msg);

  StatusCode ret = can_transmit(&msg, NULL);
  TEST_ASSERT_OK(ret);
  prv_clock_tx();

  Event e = { 0 };
  // Handle message RX
  while (event_process(&e) != STATUS_CODE_OK) {
  }
  TEST_ASSERT_EQUAL(TEST_CAN_EVENT_RX, e.id);
  bool processed = can_process_event(&e);
  TEST_ASSERT_TRUE(processed);

  TEST_ASSERT_EQUAL(msg.msg_id, rx_msg.msg_id);
  TEST_ASSERT_EQUAL(msg.data, rx_msg.data);
}
