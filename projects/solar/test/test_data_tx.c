#include "data_tx.h"

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "data_store.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "misc.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "solar_events.h"
#include "test_helpers.h"
#include "unity.h"

static uint16_t s_can_msg_count;
static uint16_t s_num_callbacks;
static uint32_t s_can_msg_values[2 * NUM_DATA_POINTS];
static DataPoint s_can_msgs_data_points[2 * NUM_DATA_POINTS];
static CanStorage s_can_storage = { 0 };
static DataTxSettings s_data_tx_settings = {
  .msgs_per_tx_iteration = 8,
  .wait_between_tx_in_millis = 150,
};

static StatusCode prv_test_data_tx_callback_handler(const CanMessage *msg, void *context,
                                                    CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_SOLAR_DATA_5_MPPTS, msg->msg_id);
  if (s_can_msg_count < 2 * NUM_DATA_POINTS) {
    CAN_UNPACK_SOLAR_DATA_5_MPPTS(msg, (uint32_t *)&s_can_msgs_data_points[s_can_msg_count],
                                  &s_can_msg_values[s_can_msg_count]);
    s_can_msg_count++;
    return STATUS_CODE_OK;
  }
  return STATUS_CODE_RESOURCE_EXHAUSTED;
}

// Set every data point
static void prv_test_data_tx_set_data_values(void) {
  for (DataPoint data_point = 0; data_point < NUM_DATA_POINTS; data_point++) {
    TEST_ASSERT_OK(data_store_set(data_point, data_point));
  }
}

static void prv_reset_arrays(void) {
  for (uint16_t index = 0; index < 2 * NUM_DATA_POINTS; index++) {
    s_can_msg_values[index] = 0;
    s_can_msgs_data_points[index] = 0;
  }
}

static void prv_process_can_events(SoftTimerId timer_id, void *context) {
  uint16_t msgs_for_this_iteration =
      MIN(s_data_tx_settings.msgs_per_tx_iteration, NUM_DATA_POINTS - s_can_msg_count);
  LOG_DEBUG("processing can messages: s_can_msg_count=%d\n", s_can_msg_count);
  for (uint16_t msg = 0; msg < msgs_for_this_iteration; msg++) {
    MS_TEST_HELPER_CAN_TX(SOLAR_CAN_EVENT_TX);
  }
  for (uint16_t msg = 0; msg < msgs_for_this_iteration; msg++) {
    MS_TEST_HELPER_CAN_RX(SOLAR_CAN_EVENT_RX);
  }
  s_num_callbacks++;
  if (s_can_msg_count < NUM_DATA_POINTS) {
    TEST_ASSERT_OK(soft_timer_start_millis(s_data_tx_settings.wait_between_tx_in_millis,
                                           prv_process_can_events, NULL, NULL));
  }
}

void setup_test(void) {
  TEST_ASSERT_OK(initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_SOLAR_5_MPPTS,
                                                 SOLAR_CAN_EVENT_TX, SOLAR_CAN_EVENT_RX,
                                                 SOLAR_CAN_EVENT_FAULT));
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_SOLAR_DATA_5_MPPTS,
                                         prv_test_data_tx_callback_handler, NULL));
  TEST_ASSERT_OK(data_store_init());

  TEST_ASSERT_OK(data_tx_init(&s_data_tx_settings));

  s_can_msg_count = 0;
  s_num_callbacks = 0;
  prv_reset_arrays();
}

void teardown_test(void) {}

// ensure no can messages are sent when data points are not set
void test_data_tx_no_values_set(void) {
  Event e = { .id = DATA_READY_EVENT };
  TEST_ASSERT(data_tx_process_event(&e));

  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

// ensure data_tx_process_event can handle NULL events or irrelevant events
void test_data_tx_invalid_event(void) {
  TEST_ASSERT_FALSE(data_tx_process_event(NULL));
  Event e = { .id = DATA_READY_EVENT + 1 };
  TEST_ASSERT_FALSE(data_tx_process_event(&e));
}

// set all data points, tx all data points, verify the values of the data point and value in the
// expected order
void test_data_tx(void) {
  prv_test_data_tx_set_data_values();
  Event e = { .id = DATA_READY_EVENT };
  TEST_ASSERT(data_tx_process_event(&e));

  prv_process_can_events(SOFT_TIMER_INVALID_TIMER, NULL);
  uint32_t delay_time = ((uint32_t)NUM_DATA_POINTS / s_data_tx_settings.msgs_per_tx_iteration + 1) *
                        s_data_tx_settings.wait_between_tx_in_millis;
  delay_ms(delay_time);
  TEST_ASSERT_EQUAL(NUM_DATA_POINTS / s_data_tx_settings.msgs_per_tx_iteration, s_num_callbacks);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_EQUAL(NUM_DATA_POINTS, s_can_msg_count);

  for (uint16_t msg = 0; msg < NUM_DATA_POINTS; msg++) {
    TEST_ASSERT_EQUAL(msg, s_can_msg_values[msg]);
    TEST_ASSERT_EQUAL(msg, s_can_msgs_data_points[msg]);
  }
}
