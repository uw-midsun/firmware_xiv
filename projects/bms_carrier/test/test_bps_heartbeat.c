#include <string.h>

#include "bms_events.h"
#include "bps_heartbeat.h"
#include "can.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"

#undef BPS_HB_FREQ_MS
#define BPS_HB_FREQ_MS 50

static CanStorage s_can_storage = { 0 };
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_BMS_CARRIER,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = BMS_CAN_EVENT_RX,
  .tx_event = BMS_CAN_EVENT_TX,
  .fault_event = BMS_CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static BpsStorage s_storage = { 0 };

static uint8_t s_hb_count = 0;
static CanAckStatus s_ack_status = NUM_CAN_ACK_STATUSES;
static uint8_t s_bps_bitmask = 0;

static StatusCode prv_hb_rx(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  s_hb_count++;

  *ack_reply = s_ack_status;

  CAN_UNPACK_BPS_HEARTBEAT(msg, &s_bps_bitmask);

  return s_ack_status == CAN_ACK_STATUS_OK ? STATUS_CODE_OK : STATUS_CODE_TIMEOUT;
}

static uint8_t s_fault_bps_bitmask = 0;
static bool s_fault_bps_clear = false;

StatusCode TEST_MOCK(fault_bps)(uint8_t fault_bitmask, bool clear) {
  s_fault_bps_bitmask = fault_bitmask;
  s_fault_bps_clear = clear;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  s_hb_count = 0;
  s_bps_bitmask = 0;
  s_ack_status = NUM_CAN_ACK_STATUSES;
  memset(&s_storage, 0, sizeof(s_storage));
  s_storage.ack_devices = SYSTEM_CAN_DEVICE_BMS_CARRIER;
  s_fault_bps_bitmask = 0;
  s_fault_bps_clear = false;
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  can_init(&s_can_storage, &s_can_settings);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_BPS_HEARTBEAT, prv_hb_rx, NULL);
}

void teardown_test(void) {}

void test_init_forces_heartbeat(void) {
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, BPS_HB_FREQ_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, s_hb_count);
}

void test_hb_repeats(void) {
  s_ack_status = CAN_ACK_STATUS_OK;
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, BPS_HB_FREQ_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(BPS_HB_FREQ_MS + 3);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(BPS_HB_FREQ_MS + 3);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(BPS_HB_FREQ_MS + 3);

  TEST_ASSERT_EQUAL(3, s_hb_count);
}

void test_good_ack_doesnt_fault(void) {
  s_ack_status = CAN_ACK_STATUS_OK;
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, BPS_HB_FREQ_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);

  TEST_ASSERT_EQUAL(0, s_storage.fault_bitset);
  TEST_ASSERT_EQUAL(0, s_storage.ack_fail_count);
}

void test_hb_fails_then_faults(void) {
  s_ack_status = CAN_ACK_STATUS_TIMEOUT;
  TEST_ASSERT_OK(bps_heartbeat_init(&s_storage, BPS_HB_FREQ_MS));
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(BPS_HB_FREQ_MS + 3);
  TEST_ASSERT_EQUAL(1, s_storage.ack_fail_count);

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(BPS_HB_FREQ_MS + 3);
  TEST_ASSERT_EQUAL(2, s_storage.ack_fail_count);

  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX);
  delay_ms(BPS_HB_FREQ_MS + 3);
  TEST_ASSERT_EQUAL(3, s_storage.ack_fail_count);

  TEST_ASSERT_EQUAL(EE_BPS_STATE_FAULT_ACK_TIMEOUT, s_fault_bps_bitmask);
  TEST_ASSERT_EQUAL(false, s_fault_bps_clear);
}
