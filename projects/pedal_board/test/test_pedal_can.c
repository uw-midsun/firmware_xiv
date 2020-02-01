#include "can.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pedal_can.h"
#include "pedal_events.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define CAN_DEVICE_ID 0x1

static CanStorage can_storage;

void setup_test(void) {
  const CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = PEDAL_CAN_RX,
    .tx_event = PEDAL_CAN_TX,
    .fault_event = PEDAL_CAN_FAULT,
    .loopback = true,
    .tx = { GPIO_PORT_A, 12 },  // CHANGE
    .rx = { GPIO_PORT_A, 11 },  // CHANGE
  };
  gpio_init();
  interrupt_init();
  gpio_it_init();
  soft_timer_init();
  event_queue_init();
  pedal_can_init(&can_storage, &can_settings);
}

void teardown_test(void) {}

void test_assert_trivial(void) {
  TEST_ASSERT_TRUE(true);
}

// Transmit a pedal can state can message, and expect the correct event to be raised.
void test_pedal_can_rx_handler_drive(void) {
  // Transmit an drive state message.
  TEST_ASSERT_OK(CAN_TRANSMIT_DRIVE_STATE(PEDAL_DRIVE_INPUT_EVENT_DRIVE));
  Event e = { 0 };
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }

  TEST_ASSERT_EQUAL(PEDAL_DRIVE_INPUT_EVENT_DRIVE, e.id);
}

void test_pedal_can_rx_handler_neutral(void) {
  // Transmit an drive state message.
  TEST_ASSERT_OK(CAN_TRANSMIT_DRIVE_STATE(PEDAL_DRIVE_INPUT_EVENT_NEUTRAL));
  Event e = { 0 };
  MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
  while (!status_ok(event_process(&e))) {
  }

  TEST_ASSERT_EQUAL(PEDAL_DRIVE_INPUT_EVENT_NEUTRAL, e.id);
}

// just to make sure can messages are correct
// so need to test pedal_can_process_event
// void test_pedal_can_rx_handler_brake_pressed(void) {
//   // raises a pedal pressed state event.
//   Event e = { 0 };
//   e.id = PEDAL_BRAKE_FSM_EVENT_PRESSED;
//   pedal_can_process_event(&e);

//   MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
//   while(!event_process(&e)){}

//   TEST_ASSERT_EQUAL(PEDAL_CAN_EVENT_BRAKE_PRESSED, e.data);
// }

// void test_pedal_can_rx_handler_brake_released(void) {
//   // Transmit a pedal released state message.
//   Event e = { 0 };
//   e.id = PEDAL_BRAKE_FSM_EVENT_RELEASED;
//   pedal_can_process_event(&e);

//   MS_TEST_HELPER_CAN_TX_RX(PEDAL_CAN_TX, PEDAL_CAN_RX);
//   while(!event_process(&e)){}
//   TEST_ASSERT_EQUAL(PEDAL_CAN_EVENT_BRAKE_RELEASED, e.data);
// }
