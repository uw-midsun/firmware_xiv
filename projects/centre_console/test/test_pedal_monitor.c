#include "pedal_rx.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "centre_console_events.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pedal_monitor.h"
#include "soft_timer.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;

void setup_test(void) {
  event_queue_init();
  gpio_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
    .bitrate = CAN_HW_BITRATE_125KBPS,
    .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
    .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
    .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  TEST_ASSERT_OK(pedal_monitor_init());
}

void teardown_test(void) {}

void test_gets_correct_states(void) {
  float throttle_float = 12.4;
  float brake_float = PEDAL_STATE_THRESHOLD + 2.5;

  uint32_t throttle_output = (uint32_t)(throttle_float * EE_PEDAL_VALUE_DENOMINATOR);
  uint32_t brake_output = (uint32_t)(brake_float * EE_PEDAL_VALUE_DENOMINATOR);
  // transmitting a message
  CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output, brake_output);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // waiting for an update
  delay_ms(PEDAL_STATE_UPDATE_FREQUENCY_MS + 1);
  TEST_ASSERT_EQUAL(PEDAL_STATE_PRESSED, get_pedal_state());

  brake_float = PEDAL_STATE_THRESHOLD - 2.5;
  brake_output = (uint32_t)(brake_float * EE_PEDAL_VALUE_DENOMINATOR);
  CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output, brake_output);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  delay_ms(PEDAL_STATE_UPDATE_FREQUENCY_MS + 1);
  TEST_ASSERT_EQUAL(PEDAL_STATE_RELEASED, get_pedal_state());
}

void test_timeout_gives_released(void) {
  float throttle_float = 12.4;
  float brake_float = PEDAL_STATE_THRESHOLD + 2.5;

  uint32_t throttle_output = (uint32_t)(throttle_float * EE_PEDAL_VALUE_DENOMINATOR);
  uint32_t brake_output = (uint32_t)(brake_float * EE_PEDAL_VALUE_DENOMINATOR);
  CAN_TRANSMIT_PEDAL_OUTPUT(throttle_output, brake_output);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  delay_ms(PEDAL_STATE_UPDATE_FREQUENCY_MS + 1);
  TEST_ASSERT_EQUAL(PEDAL_STATE_PRESSED, get_pedal_state());

  delay_ms(PEDAL_RX_TIMEOUT_MS);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, PEDAL_MONITOR_RX_TIMED_OUT, 0);
  TEST_ASSERT_EQUAL(PEDAL_STATE_RELEASED, get_pedal_state());
}
