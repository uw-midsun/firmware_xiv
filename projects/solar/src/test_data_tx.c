#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "data_store.h"
#include "data_tx.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

int count = -1;                 // count of can msgs sent, incremented in rx_handler
bool changeable_is_set = true;  // value that tells data_tx if data points are set
static CanMessage s_can_msg;    // variable used to hold can msgs and verify values

bool TEST_MOCK(data_store_get_is_set)(DataPoint data_point, bool *is_set) {
  return changeable_is_set;
}

StatusCode TEST_MOCK(data_store_get)(DataPoint data_point, uint32_t *value) {
  *value = data_point;
  return STATUS_CODE_OK;
}

typedef enum { TEST_DATA_TX_EVENT_CAN_TX = 0, TEST_DATA_TX_EVENT_CAN_RX } TestDataTxEvent;

static CanStorage s_can_storage = { 0 };
static CanSettings s_can_settings = {
  .device_id = SYSTEM_CAN_DEVICE_SOLAR,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = TEST_DATA_TX_EVENT_CAN_RX,
  .tx_event = TEST_DATA_TX_EVENT_CAN_TX,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

StatusCode prv_test_data_tx_callback_handler(const CanMessage *msg, void *context,
                                             CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_SOLAR_DATA, msg->msg_id);
  s_can_msg = *msg;
  count++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));

  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_DATA,
                                         prv_test_data_tx_callback_handler, NULL));
}

void teardown_test(void) {}

// raise DataReadyEvent and data_tx will tx all data points into CAN, verify can message values and
// count
void test_data_tx(void) {
  event_raise(DATA_READY_EVENT, 0);
  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(changeable_is_set, true);
  data_tx_process_event(&e);

  for (int can_msg = 0; can_msg < NUM_DATA_POINTS; can_msg++) {
    MS_TEST_HELPER_CAN_TX_RX(TEST_DATA_TX_EVENT_CAN_TX, TEST_DATA_TX_EVENT_CAN_RX);
    TEST_ASSERT_EQUAL(count, can_msg);
    TEST_ASSERT_EQUAL(s_can_msg.data_u32[0], count);
    TEST_ASSERT_EQUAL(s_can_msg.data_u32[1], count);
  }
}

// ensure no can messages are sent when data points are not set
void test_data_tx_no_values_set(void) {
  count = 0;
  changeable_is_set = false;
  event_raise(DATA_READY_EVENT, 0);
  Event e = { 0 };
  MS_TEST_HELPER_AWAIT_EVENT(e);
  TEST_ASSERT_EQUAL(changeable_is_set, false);
  TEST_ASSERT_EQUAL(count, 0);
  data_tx_process_event(&e);

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(count, 0);
}
