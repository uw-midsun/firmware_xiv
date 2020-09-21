#include "can_handler.h"

#include "bms.h"
#include "bms_events.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "delay.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "test_helpers.h"
#include "unity.h"

#define TEST_CELL_VOLTAGE 5
#define TEST_AVG_CURRENT 6
#define TEST_RELAY_STATE 1
#define TOTAL_CAN_MESSAGES (NUM_TOTAL_CELLS + 2)
#define TIME_BETWEEN_TX_IN_MILLIS 100
// #define TEST_FAN_STATUS STATUS_CODE_INTERNAL_ERROR

static uint16_t s_can_msg_count;
static uint16_t s_can_msg_voltage_values[NUM_TOTAL_CELLS];
static uint16_t s_can_msg_temp_values[NUM_TOTAL_CELLS];
static BmsStorage s_bms_storage;
static CanStorage s_can_storage = { 0 };

static StatusCode prv_test_can_handler_current_tx_callback_handler(const CanMessage *msg,
                                                                   void *context,
                                                                   CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC, msg->msg_id);
  uint32_t avg_current;
  uint32_t avg_voltage;
  CAN_UNPACK_BATTERY_AGGREGATE_VC(msg, &avg_voltage, &avg_current);
  TEST_ASSERT_EQUAL(TEST_CELL_VOLTAGE, avg_voltage);
  TEST_ASSERT_EQUAL(TEST_AVG_CURRENT, avg_current);
  s_can_msg_count++;
  return STATUS_CODE_OK;
}

static StatusCode prv_test_can_handler_voltage_and_tx_callback_handler(const CanMessage *msg,
                                                                       void *context,
                                                                       CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_BATTERY_VT, msg->msg_id);
  uint16_t module_id;
  uint16_t voltage;
  uint16_t temp;
  CAN_UNPACK_BATTERY_VT(msg, &module_id, &voltage, &temp);
  if (module_id < NUM_TOTAL_CELLS) {
    s_can_msg_voltage_values[module_id] = voltage;
    s_can_msg_temp_values[module_id] = temp;
    s_can_msg_count++;
    return STATUS_CODE_OK;
  }
  return STATUS_CODE_RESOURCE_EXHAUSTED;
}

static StatusCode prv_test_can_handler_relay_state_tx_callback_handler(const CanMessage *msg,
                                                                       void *context,
                                                                       CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_BATTERY_RELAY_STATE, msg->msg_id);
  uint8_t hv_enabled;
  uint8_t gnd_enabled;
  CAN_UNPACK_BATTERY_RELAY_STATE(msg, &hv_enabled, &gnd_enabled);
  TEST_ASSERT_EQUAL(TEST_RELAY_STATE, hv_enabled);
  TEST_ASSERT_EQUAL(TEST_RELAY_STATE, gnd_enabled);
  s_can_msg_count++;
  return STATUS_CODE_OK;
}

/*
static StatusCode prv_test_can_handler_fan_status_tx_callback_handler(const CanMessage *msg, void
*context, CanAckStatus *ack_reply) { TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_BATTERY_FAN_STATE,
msg->msg_id);
  


  s_can_msg_count++;
  return STATUS_CODE_OK;
}
*/

static void prv_set_bms_storage(void) {
  s_bms_storage.current_storage.average = TEST_AVG_CURRENT;
  s_bms_storage.relay_storage.gnd_enabled = TEST_RELAY_STATE;
  s_bms_storage.relay_storage.hv_enabled = TEST_RELAY_STATE;
  // s_bms_storage.fan_storage.status = TEST_FAN_STATUS;

  for (uint8_t cell = 0; cell < NUM_TOTAL_CELLS; cell++) {
    s_bms_storage.afe_readings.voltages[cell] = TEST_CELL_VOLTAGE;
  }

  for (uint8_t thermistor = 0; thermistor < NUM_THERMISTORS; thermistor++) {
    s_bms_storage.afe_readings.temps[thermistor] = thermistor;
  }
}

static void prv_process_can_events(SoftTimerId timer_id, void *context) {
  MS_TEST_HELPER_CAN_TX(BMS_CAN_EVENT_TX);
  LOG_DEBUG("did a process tx \n");
  MS_TEST_HELPER_CAN_RX(BMS_CAN_EVENT_RX);
  LOG_DEBUG("did a process rx \n");
  if (s_can_msg_count < TOTAL_CAN_MESSAGES) {
    soft_timer_start_millis(TIME_BETWEEN_TX_IN_MILLIS, prv_process_can_events, NULL, NULL);
  }
}

void setup_test(void) {
  TEST_ASSERT_OK(initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_BMS_CARRIER,
                                                 BMS_CAN_EVENT_TX, BMS_CAN_EVENT_RX,
                                                 BMS_CAN_EVENT_FAULT));
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BATTERY_AGGREGATE_VC,
                                         prv_test_can_handler_current_tx_callback_handler, NULL));
  TEST_ASSERT_OK(can_register_rx_handler(
      SYSTEM_CAN_MESSAGE_BATTERY_VT, prv_test_can_handler_voltage_and_tx_callback_handler, NULL));
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BATTERY_RELAY_STATE,
                                         prv_test_can_handler_relay_state_tx_callback_handler,
                                         NULL));
  // TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_BATTERY_FAN_STATE,
  // prv_test_can_handler_fan_status_tx_callback_handler, NULL));

  prv_set_bms_storage();
}

void teardown_test(void) {}

void test_can_handler_invalid_args(void) {
  TEST_ASSERT_EQUAL(STATUS_CODE_INVALID_ARGS, can_handler_init(NULL));
}

void test_can_handler(void) {
  TEST_ASSERT_OK(can_handler_init(&s_bms_storage));
  prv_process_can_events(SOFT_TIMER_INVALID_TIMER, NULL);
  delay_ms((TOTAL_CAN_MESSAGES + 2) * TIME_BETWEEN_TX_IN_MILLIS);

  TEST_ASSERT_EQUAL(TOTAL_CAN_MESSAGES, s_can_msg_count);
  for (uint8_t cell = 0; cell < NUM_TOTAL_CELLS; cell++) {
    TEST_ASSERT_EQUAL(TEST_CELL_VOLTAGE, s_can_msg_voltage_values[cell]);
    TEST_ASSERT_EQUAL(1, s_can_msg_temp_values[cell] % 2);  // Ensure all temp values are odd
  }
}
