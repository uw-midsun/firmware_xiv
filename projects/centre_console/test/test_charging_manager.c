#include "can_transmit.h"
#include "can_unpack.h"
#include "centre_console_events.h"
#include "charging_manager.h"
#include "drive_fsm.h"
#include "exported_enums.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"

#define TEST_CAN_DEVICE_ID 0x1

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
  .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
  .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static DriveState s_drive_state;

void setup_test(void) {
  s_drive_state = NUM_DRIVE_STATES;
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);
  TEST_ASSERT_OK(init_charging_manager(&s_drive_state));
}

void teardown_test(void) {}

void test_get_global_charging_state(void) {
  // should initialize to not charging
  ChargingState charging_state = get_global_charging_state();
  TEST_ASSERT_EQUAL(CHARGING_STATE_NOT_CHARGING, charging_state);
}

void test_permission_grant(void) {
  s_drive_state = DRIVE_STATE_PARKING;
  ChargingState charging_state = get_global_charging_state();
  TEST_ASSERT_EQUAL(CHARGING_STATE_NOT_CHARGING, charging_state);

  // request permission
  CAN_TRANSMIT_REQUEST_TO_CHARGE();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // should be charging now
  charging_state = get_global_charging_state();
  TEST_ASSERT_EQUAL(CHARGING_STATE_CHARGING, charging_state);
}

void test_disconnect_rx_handler(void) {
  s_drive_state = DRIVE_STATE_PARKING;
  // start charging
  CAN_TRANSMIT_REQUEST_TO_CHARGE();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  // tx and rx the allow charging event
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // charger should set state to not charging if charger is disconnected
  CAN_TRANSMIT_CHARGER_CONNECTED_STATE(EE_CHARGER_CONN_STATE_DISCONNECTED);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  ChargingState charging_state = get_global_charging_state();
  TEST_ASSERT_EQUAL(CHARGING_STATE_NOT_CHARGING, charging_state);

  // charger should not set state to charging if charger is only connected
  CAN_TRANSMIT_CHARGER_CONNECTED_STATE(EE_CHARGER_CONN_STATE_CONNECTED);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  charging_state = get_global_charging_state();
  TEST_ASSERT_EQUAL(CHARGING_STATE_NOT_CHARGING, charging_state);
}

void test_permission_deny(void) {
  s_drive_state = DRIVE_STATE_NEUTRAL;
  ChargingState charging_state = get_global_charging_state();
  TEST_ASSERT_EQUAL(CHARGING_STATE_NOT_CHARGING, charging_state);

  // request permission
  CAN_TRANSMIT_REQUEST_TO_CHARGE();
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // should be not charging
  charging_state = get_global_charging_state();
  TEST_ASSERT_EQUAL(CHARGING_STATE_NOT_CHARGING, charging_state);
}
