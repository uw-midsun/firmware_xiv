#include "battery_monitor.h"
#include "can_transmit.h"
#include "charger_events.h"
#include "ms_test_helpers.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_DEVICE_ID 0x1

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = TEST_CAN_DEVICE_ID,         //
  .bitrate = CAN_HW_BITRATE_250KBPS,       //
  .tx = { GPIO_PORT_A, 12 },               //
  .rx = { GPIO_PORT_A, 11 },               //
  .rx_event = CHARGER_CAN_EVENT_RX,        //
  .tx_event = CHARGER_CAN_EVENT_TX,        //
  .fault_event = CHARGER_CAN_EVENT_FAULT,  //
  .loopback = true                         //
};

void setup_test(void) {
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);
  TEST_ASSERT_OK(battery_monitor_init());
}

void teardown_test(void) {}

void test_normal_rx(void) {
  const uint32_t happy_voltage = 1300;

  CAN_TRANSMIT_BATTERY_AGGREGATE_VC(happy_voltage, 0);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void test_overvolt_rx(void) {
  const uint32_t unhappy_voltage = 1400;

  CAN_TRANSMIT_BATTERY_AGGREGATE_VC(unhappy_voltage, 0);
  MS_TEST_HELPER_CAN_TX_RX(CHARGER_CAN_EVENT_TX, CHARGER_CAN_EVENT_RX);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CHARGER_STOP_FSM_EVENT_BEGIN, 0);
}
