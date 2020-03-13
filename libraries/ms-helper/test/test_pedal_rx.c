#include "pedal_rx.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_PEDAL_RX_CAN_DEVICE_ID 12

typedef enum {
  TEST_PEDAL_RX_CAN_RX = 0,
  TEST_PEDAL_RX_CAN_TX,
  TEST_PEDAL_RX_CAN_FAULT,
  TEST_PEDAL_RX_TIMEOUT
} TestPedalCanEvent;

#define TEST_PEDAL_RX_VALUE_THRESHOLD 0.01f
#define TIMEOUT_MS 50

static CanStorage s_can_storage;
static PedalRxStorage s_pedal_rx_storage;
static PedalRxSettings s_pedal_rx_settings = {
  .timeout_event = TEST_PEDAL_RX_TIMEOUT,
  .timeout_ms = TIMEOUT_MS,
};

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_PEDAL_RX_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = TEST_PEDAL_RX_CAN_RX,
    .tx_event = TEST_PEDAL_RX_CAN_TX,
    .fault_event = TEST_PEDAL_RX_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  TEST_ASSERT_OK(pedal_rx_init(&s_pedal_rx_storage, &s_pedal_rx_settings));
}

void teardown_test(void) {}

void test_pedal_rx_receives_correct_values(void) {
  float throttle_float = 12.4;
  float brake_float = 13.26;

  uint32_t throttle_output = (uint32_t)(throttle_float * EE_PEDAL_VALUE_DENOMINATOR);
  uint32_t brake_output = (uint32_t)(brake_float * EE_PEDAL_VALUE_DENOMINATOR);
  // transmitting a message
  CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output, brake_output);
  MS_TEST_HELPER_CAN_TX_RX(TEST_PEDAL_RX_CAN_TX, TEST_PEDAL_RX_CAN_RX);

  // making sure it received the correct values
  PedalValues values = pedal_rx_get_pedal_values(&s_pedal_rx_storage);
  TEST_ASSERT_EQUAL(brake_float, values.brake);
  TEST_ASSERT_EQUAL(throttle_float, values.throttle);
}

void test_pedal_rx_timeout_goes_off_if_pedal_messages_are_not_received_fast_enough(void) {
  float throttle_float = 12.4;
  float brake_float = 13.26;

  uint32_t throttle_output = (uint32_t)(throttle_float * EE_PEDAL_VALUE_DENOMINATOR);
  uint32_t brake_output = (uint32_t)(brake_float * EE_PEDAL_VALUE_DENOMINATOR);

  // we want to try at least twice to make sure the module clears its own fault
  for (uint8_t i = 0; i < 2; i++) {
    // transmitting a message
    CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output, brake_output);
    MS_TEST_HELPER_CAN_TX_RX(TEST_PEDAL_RX_CAN_TX, TEST_PEDAL_RX_CAN_RX);

    PedalValues values = pedal_rx_get_pedal_values(&s_pedal_rx_storage);
    TEST_ASSERT_EQUAL(brake_float, values.brake);
    TEST_ASSERT_EQUAL(throttle_float, values.throttle);

    // delaying for more than timeout
    delay_ms(TIMEOUT_MS - 10);
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

    delay_ms(20);
    Event e = { 0 };
    MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, TEST_PEDAL_RX_TIMEOUT, 0);

    // values should be reported as 0
    values = pedal_rx_get_pedal_values(&s_pedal_rx_storage);
    TEST_ASSERT_EQUAL(0, values.brake);
    TEST_ASSERT_EQUAL(0, values.throttle);

    // no further events should be raised
    delay_ms(TIMEOUT_MS + 10);
    MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  }
}
