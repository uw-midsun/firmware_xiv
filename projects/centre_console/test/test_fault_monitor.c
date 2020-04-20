#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "delay.h"
#include "exported_enums.h"
#include "fault_monitor.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "unity.h"

static CanStorage s_can_storage = { 0 };
static bool s_callback_acked = false;

#define TEST_FAULT_MONITOR_TIMEOUT 100

void setup_test(void) {
  TEST_ASSERT_OK(initialize_can_in_test(&s_can_storage, SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
                                        CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX,
                                        CENTRE_CONSOLE_EVENT_CAN_FAULT));
  fault_monitor_init(TEST_FAULT_MONITOR_TIMEOUT);
  s_callback_acked = false;
  *get_fault_status() = NUM_FAULT_STATUS;
}

void teardown_test(void) {}

static StatusCode prv_ack_cb(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                             uint16_t num_remaining, void *context) {
  s_callback_acked = true;
  return STATUS_CODE_OK;
}

void test_fault_monitor_sets_the_status_if_theres_a_fault_and_clearas_it(void) {
  TEST_ASSERT_EQUAL(*get_fault_status(), NUM_FAULT_STATUS);

  uint8_t bps_fault_bitset = EE_BATTERY_HEARTBEAT_STATE_FAULT_KILLSWITCH |
                             EE_BATTERY_HEARTBEAT_STATE_FAULT_CURRENT_SENSE_AFE_CELL;
  CanAckRequest s_req = { .callback = prv_ack_cb,
                          .context = NULL,
                          .expected_bitset =
                              CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE) };
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, bps_fault_bitset);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_EQUAL(*get_fault_status(), FAULT_STATUS_FAULT);
  TEST_ASSERT_TRUE(s_callback_acked);
  s_callback_acked = false;

  uint8_t fault_cleared = 0;

  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, fault_cleared);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(*get_fault_status(), FAULT_STATUS_OK);
  TEST_ASSERT_TRUE(s_callback_acked);
}

void test_fault_monitor_times_out(void) {
  TEST_ASSERT_EQUAL(*get_fault_status(), NUM_FAULT_STATUS);
  uint8_t bps_fault_bitset = EE_BATTERY_HEARTBEAT_STATE_FAULT_KILLSWITCH |
                             EE_BATTERY_HEARTBEAT_STATE_FAULT_CURRENT_SENSE_AFE_CELL;
  CanAckRequest s_req = { .callback = prv_ack_cb,
                          .context = NULL,
                          .expected_bitset =
                              CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE) };
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, bps_fault_bitset);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_EQUAL(FAULT_STATUS_FAULT, *get_fault_status());

  delay_ms(TEST_FAULT_MONITOR_TIMEOUT + 5);

  TEST_ASSERT_EQUAL(*get_fault_status(), FAULT_STATUS_ACK_FAULT);

  uint8_t fault_cleared = 0;
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, fault_cleared);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);

  TEST_ASSERT_EQUAL(*get_fault_status(), FAULT_STATUS_OK);

  TEST_ASSERT_TRUE(s_callback_acked);
}
