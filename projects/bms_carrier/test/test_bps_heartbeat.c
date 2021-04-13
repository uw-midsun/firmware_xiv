#include <string.h>
#include <time.h>
#include "bms_events.h"
#include "bps_heartbeat.h"
#include "can.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"

#define TEST_BPS_HEARTBEAT_PERIOD_MS 100
#define BUFFER_TIME_MS 3

static CanStorage s_can_storage = { 0 };

static BpsStorage s_storage = { 0 };

static uint8_t s_hb_count = 0;
static CanAckStatus s_ack_status = NUM_CAN_ACK_STATUSES;
static uint8_t s_bps_bitmask = 0;
static uint8_t s_fault_bps_bitmask = 0;
static bool s_fault_bps_clear = false;

static StatusCode prv_hb_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  s_hb_count++;

  *ack_reply = s_ack_status;

  CAN_UNPACK_BPS_HEARTBEAT(msg, &s_bps_bitmask);

  return s_ack_status == CAN_ACK_STATUS_OK ? STATUS_CODE_OK : STATUS_CODE_TIMEOUT;
}

StatusCode TEST_MOCK(fault_bps_set)(uint8_t fault_bitmask) {
  s_fault_bps_bitmask = fault_bitmask;
  s_fault_bps_clear = false;
  return STATUS_CODE_OK;
}

StatusCode TEST_MOCK(fault_bps_clear)(uint8_t fault_bitmask) {
  s_fault_bps_bitmask = fault_bitmask;
  s_fault_bps_clear = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BMS_CARRIER, BMS_CAN_EVENT_TX,
                                  BMS_CAN_EVENT_RX, BMS_CAN_EVENT_FAULT);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_hb_rx, NULL);
  s_hb_count = 0;
  s_bps_bitmask = 0;
  s_ack_status = NUM_CAN_ACK_STATUSES;
  memset(&s_storage, 0, sizeof(s_storage));
  s_storage.ack_devices = SYSTEM_CAN_DEVICE_BMS_CARRIER;
  s_fault_bps_bitmask = 0;
  s_fault_bps_clear = false;
}

void teardown_test(void) {}

void test_init_forces_heartbeat(void) {
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, TEST_BPS_HEARTBEAT_PERIOD_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, s_hb_count);
}

void test_hb_repeats(void) {
  s_ack_status = CAN_ACK_STATUS_OK;
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, TEST_BPS_HEARTBEAT_PERIOD_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(TEST_BPS_HEARTBEAT_PERIOD_MS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(TEST_BPS_HEARTBEAT_PERIOD_MS);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(TEST_BPS_HEARTBEAT_PERIOD_MS);

  TEST_ASSERT_EQUAL(3, s_hb_count);
}

void test_good_ack_doesnt_fault(void) {
  s_ack_status = CAN_ACK_STATUS_OK;
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, TEST_BPS_HEARTBEAT_PERIOD_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(0, s_storage.fault_bitset);
  TEST_ASSERT_EQUAL(0, s_storage.ack_fail_count);
}

void test_hb_fails_then_faults(void) {
  s_ack_status = CAN_ACK_STATUS_TIMEOUT;
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, TEST_BPS_HEARTBEAT_PERIOD_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  TEST_ASSERT_OK(
      time_assert(1, (&s_storage.ack_fail_count), TEST_BPS_HEARTBEAT_PERIOD_MS, BUFFER_TIME_MS));

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  TEST_ASSERT_OK(
      time_assert(2, (&s_storage.ack_fail_count), TEST_BPS_HEARTBEAT_PERIOD_MS, BUFFER_TIME_MS));

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  TEST_ASSERT_OK(
      time_assert(3, (&s_storage.ack_fail_count), TEST_BPS_HEARTBEAT_PERIOD_MS, BUFFER_TIME_MS));

  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_ACK_TIMEOUT, s_fault_bps_bitmask);
  TEST_ASSERT_EQUAL(false, s_fault_bps_clear);
}
