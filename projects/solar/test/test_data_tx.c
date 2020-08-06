#include "data_tx.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "data_store.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

static uint16_t s_can_msg_count;
static CanMessage s_can_msg;
static CanStorage s_can_storage = { 0 };

typedef enum {
  TEST_DATA_TX_EVENT_CAN_TX = 0,  //
  TEST_DATA_TX_EVENT_CAN_RX,      //
  TEST_DATA_TX_EVENT_CAN_FAULT,   //
  NUM_TEST_DATA_TX_CAN_EVENTS     //
} TestDataTxEvent;

static StatusCode prv_test_data_tx_callback_handler(const CanMessage *msg, void *context,
                                                    CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_SOLAR_DATA, msg->msg_id);
  s_can_msg = *msg;
  count++;
  return STATUS_CODE_OK;
}

static void prv_test_data_tx_set_data_values(void) {
  uint32_t data_value = 0;
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    TEST_ASSERT_OK(data_store_set(data_point, data_value));
    data_value++;
  }
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  data_store_init();

  s_can_msg_count = -1;

  const CanSettings can_settings = {
    .device_id = SYSTEM_CAN_DEVICE_SOLAR,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_DATA_TX_EVENT_CAN_RX,
    .tx_event = TEST_DATA_TX_EVENT_CAN_TX,
    .fault_event = TEST_DATA_TX_EVENT_CAN_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));

  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_DATA,
                                         prv_test_data_tx_callback_handler, NULL));
}

void teardown_test(void) {}

// raise DataReadyEvent and data_tx will tx all data points into CAN
// Verify can message values and message count
void test_data_tx(void) {
  prv_test_data_tx_set_data_values();
  Event e = { .id = DATA_READY_EVENT };
  TEST_ASSERT(data_tx_process_event(&e));

  for (uint16_t can_msg = 0; can_msg < NUM_DATA_POINTS; can_msg++) {
    MS_TEST_HELPER_CAN_TX_RX(TEST_DATA_TX_EVENT_CAN_TX, TEST_DATA_TX_EVENT_CAN_RX);
    TEST_ASSERT_EQUAL(s_can_msg_count, can_msg);
    TEST_ASSERT_EQUAL(s_can_msg.data_u32[0], s_can_msg_count);
    TEST_ASSERT_EQUAL(s_can_msg.data_u32[1], s_can_msg_count);
  }
}

// ensure no can messages are sent when data points are not set
void test_data_tx_no_values_set(void) {
  Event e = { .id = DATA_READY_EVENT };
  TEST_ASSERT(data_tx_process_event(&e));

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// ensure data_tx_process_event can handle NULL events or irrelevant events
void test_data_tx_invalid_event(void) {
  TEST_ASSERT_FALSE(data_tx_process_event(NULL));
  Event e = { .id = SOLAR_FAULT_EVENT_MPPT_OVERCURRENT };
  TEST_ASSERT_FALSE(data_tx_process_event(&e));
}
