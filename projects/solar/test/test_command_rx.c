#include "command_rx.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "solar_events.h"
#include "status.h"
#include "test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage;

static uint8_t s_times_relay_closed;
static uint8_t s_times_relay_opened;

StatusCode TEST_MOCK(relay_fsm_close)(void) {
  s_times_relay_closed++;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(relay_fsm_open)(void) {
  s_times_relay_opened++;
  return STATUS_CODE_OK;
}

static void prv_transmit_command(EERelayState relay_state) {
  CAN_TRANSMIT_SET_RELAY_STATES(NULL, 1 << EE_RELAY_ID_BATTERY, relay_state << EE_RELAY_ID_BATTERY);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR_5_MPPTS,
                                  SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX, SOLAR_CAN_EVENT_FAULT);
  TEST_ASSERT_OK(command_rx_init());
  s_times_relay_closed = 0;
  s_times_relay_opened = 0;
}
void teardown_test(void) {}

// Test that sending the CAN message results in the relay FSM being triggered to open/close.
void test_open_close_can_commands(void) {
  uint8_t expected_times_relay_closed = 0;
  uint8_t expected_times_relay_opened = 0;

  prv_transmit_command(EE_RELAY_STATE_CLOSE);
  expected_times_relay_closed++;
  TEST_ASSERT_EQUAL(expected_times_relay_closed, s_times_relay_closed);
  TEST_ASSERT_EQUAL(expected_times_relay_opened, s_times_relay_opened);

  prv_transmit_command(EE_RELAY_STATE_OPEN);
  expected_times_relay_opened++;
  TEST_ASSERT_EQUAL(expected_times_relay_closed, s_times_relay_closed);
  TEST_ASSERT_EQUAL(expected_times_relay_opened, s_times_relay_opened);

  prv_transmit_command(EE_RELAY_STATE_OPEN);
  expected_times_relay_opened++;
  TEST_ASSERT_EQUAL(expected_times_relay_closed, s_times_relay_closed);
  TEST_ASSERT_EQUAL(expected_times_relay_opened, s_times_relay_opened);

  prv_transmit_command(EE_RELAY_STATE_CLOSE);
  expected_times_relay_closed++;
  TEST_ASSERT_EQUAL(expected_times_relay_closed, s_times_relay_closed);
  TEST_ASSERT_EQUAL(expected_times_relay_opened, s_times_relay_opened);
}

// Test that non-relay state CAN messages or non-battery directed relay state messages are ignored.
void test_irrelevant_can_message(void) {
  CAN_TRANSMIT_SOLAR_FAULT_5_MPPTS(0, 0);  // irrelevant CAN message
  MS_TEST_HELPER_CAN_TX_RX(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(0, s_times_relay_closed);
  TEST_ASSERT_EQUAL(0, s_times_relay_opened);

  CAN_TRANSMIT_SET_RELAY_STATES(NULL, 1 << (EE_RELAY_ID_BATTERY + 1), 0);  // not for battery
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(0, s_times_relay_closed);
  TEST_ASSERT_EQUAL(0, s_times_relay_opened);
}
