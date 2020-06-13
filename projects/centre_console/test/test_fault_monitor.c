#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "delay.h"
#include "exported_enums.h"
#include "fault_monitor.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage = { 0 };
static bool s_callback_acked = false;

#define TEST_FAULT_MONITOR_TIMEOUT_MS 25

void setup_test(void) {
  TEST_ASSERT_OK(initialize_can_and_dependencies(
      &s_can_storage, SYSTEM_CAN_DEVICE_CENTRE_CONSOLE, CENTRE_CONSOLE_EVENT_CAN_TX,
      CENTRE_CONSOLE_EVENT_CAN_RX, CENTRE_CONSOLE_EVENT_CAN_FAULT));
  fault_monitor_init(TEST_FAULT_MONITOR_TIMEOUT_MS);
  s_callback_acked = false;
}

void teardown_test(void) {}

static StatusCode prv_ack_cb(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                             uint16_t num_remaining, void *context) {
  s_callback_acked = true;
  return STATUS_CODE_OK;
}

void test_fault_monitor_faults_car(void) {
  uint8_t bps_fault_bitset = EE_BATTERY_HEARTBEAT_STATE_FAULT_KILLSWITCH |
                             EE_BATTERY_HEARTBEAT_STATE_FAULT_CURRENT_SENSE_AFE_CELL;
  CanAckRequest s_req = { .callback = prv_ack_cb,
                          .context = NULL,
                          .expected_bitset =
                              CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE) };
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, bps_fault_bitset);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // assert the fault event
  Event e = { 0 };
  FaultReason fault = { .fields = { .area = EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT, .reason = 0 } };
  MS_TEST_HELPER_ASSERT_EVENT(e, CENTRE_CONSOLE_POWER_EVENT_FAULT, fault.raw);

  // assert the ack
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_TRUE(s_callback_acked);
}

void test_fault_monitor_times_out(void) {
  uint8_t bps_fault_bitset = 0;
  CanAckRequest s_req = { .callback = prv_ack_cb,
                          .context = NULL,
                          .expected_bitset =
                              CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE) };
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, bps_fault_bitset);
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // assert no fault event, just ack
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  // assert fault event
  delay_ms(TEST_FAULT_MONITOR_TIMEOUT_MS + 5);
  Event e = { 0 };
  FaultReason fault = { .fields = { .area = EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT, .reason = 0 } };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, CENTRE_CONSOLE_POWER_EVENT_FAULT, fault.raw);
}
