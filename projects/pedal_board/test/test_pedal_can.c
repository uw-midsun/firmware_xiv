#include "can.h"
#include "event_queue.h"
#include "events.h"
#include "log.h"
#include "pedal_can.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define CAN_DEVICE_ID 0x1

static CanStorage can_storage;

void setup_test(void) {
  const CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CAN_RX,
    .tx_event = CAN_TX,
    .fault_event = CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },  // CHANGE
    .rx = { GPIO_PORT_A, 11 },  // CHANGE
  };
  TEST_ASSERT_OK(pedal_can_init(&can_storage, &can_settings));
}

void teardown_test(void) {}

void test_assert_trivial(void) {
  TEST_ASSERT_TRUE(true);
}

// Transmit a pedal can state can message, and expect the correct event to be raised.
void test_pedal_can_rx_handler(void) {
  // Transmit a pedal pressed state message.
  CanMessage msg = {
    .msg_id = 0x1,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 1,
    .data = 1,
  };
  can_transmit(&msg, NULL);
  Event e = { 0 };
  //MS_TEST_HELPER_CAN_TX_RX(CAN_TX, CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(CAN_BRAKE_PRESSED, e.id);

  // Transmit a pedal released state message.
  msg.data = 2;
  can_transmit(&msg, NULL);
  //MS_TEST_HELPER_CAN_TX_RX(CAN_TX, CAN_RX);
  while (!status_ok(event_process(&e))) {
  }
  TEST_ASSERT_EQUAL(CAN_BRAKE_RELEASED, e.id);
}

//
void test_pedal_can_brake_pressed_can(void) {
  Event e = {
    .id = CAN_BRAKE_PRESSED,
  };
  TEST_ASSERT_TRUE(pedal_can_process_event(&e));
}

void test_pedal_can_brake_released_can(void) {
  Event e = {
    .id = CAN_BRAKE_RELEASED,
  };
  TEST_ASSERT_TRUE(pedal_can_process_event(&e));
}

void test_pedal_can_rx_can(void) {
  Event e = {
    .id = CAN_RX,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_tx_can(void) {
  Event e = {
    .id = CAN_TX,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_fault_can(void) {
  Event e = {
    .id = CAN_FAULT,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_car_input_fault(void) {
  Event e = {
    .id = CAR_INPUT_FAULT,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_car_input_neutral(void) {
  Event e = {
    .id = CAR_INPUT_NEUTRAL,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_car_input_drive(void) {
  Event e = {
    .id = CAR_INPUT_DRIVE,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_brake_pressed(void) {
  Event e = {
    .id = BRAKE_PRESSED,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_brake_released(void) {
  Event e = {
    .id = BRAKE_RELEASED,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}

void test_pedal_can_throttle(void) {
  Event e = {
    .id = PEDAL_EVENT_THROTTLE_READING,
  };
  TEST_ASSERT_FALSE(pedal_can_process_event(&e));
}