#include "bps_watcher.h"
#include "can.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "pd_events.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CAN_DEVICE_ID 0x1

typedef enum {
  TEST_CAN_EVENT_RX = 1,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static CanStorage s_can_storage;
static bool s_callback_acked = false;

static StatusCode prv_ack_cb(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                             uint16_t num_remaining, void *context) {
  s_callback_acked = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  event_queue_init();

  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },  // probably fine for testing
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
  bps_watcher_init();
  s_callback_acked = false;
}
void teardown_test(void) {}

// test to make sure bps watcher does nothing if no fault
void test_bps_happy_watcher(void) {
  uint8_t bps_fault_bitset = 0;
  CanAckRequest s_req = { .callback = prv_ack_cb,
                          .context = NULL,
                          .expected_bitset =
                              CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE) };
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, bps_fault_bitset);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  // assert the no event raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// test to make sure bps watcher catches the BPS HEARTBEAT
void test_bps_fault_watcher(void) {
  uint8_t bps_fault_bitset = 0;
  CanAckRequest s_req = { .callback = prv_ack_cb,
                          .context = NULL,
                          .expected_bitset =
                              CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_CENTRE_CONSOLE) };
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, bps_fault_bitset);
  MS_TEST_HELPER_CAN_TX_RX_WITH_ACK(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  // assert the no event raised
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();

  // set fault bit
  bps_fault_bitset = 1;
  CAN_TRANSMIT_BPS_HEARTBEAT(&s_req, bps_fault_bitset);
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);

  // assert that these events are raised
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_STROBE_EVENT, 1);
  MS_TEST_HELPER_ASSERT_EVENT(e, POWER_DISTRIBUTION_POWER_SEQUENCE_EVENT_TURN_ON_EVERYTHING_AUX, 1);
  // not needed but for the ack
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
}
