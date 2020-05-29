#include "adc.h"
#include "adc_periodic_reader.h"
#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"
#include "steering_control_stalk.h"
#include "steering_digital_input.h"
#include "steering_events.h"
#include "test_helpers.h"

#define INVALID_VOLTAGE 6000
#define TIMER_INTERVAL_MS 10

static CanSettings can_settings = {
  .device_id = STEERING_CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = STEERING_CAN_EVENT_RX,
  .tx_event = STEERING_CAN_EVENT_TX,
  .fault_event = STEERING_CAN_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = true,
};

static CanStorage s_can_storage;

static int count = 0;

StatusCode prv_test_signal_rx_cb_handler(const CanMessage *msg, void *context,
                                         CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_LIGHTS, msg->msg_id);
  count++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  adc_init(ADC_MODE_SINGLE);
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  TEST_ASSERT_OK(steering_digital_input_init());
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  TEST_ASSERT_OK(adc_periodic_reader_init(TIMER_INTERVAL_MS));
  TEST_ASSERT_OK(control_stalk_init());
}

void test_control_stalk_left_signal() {
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, prv_test_signal_rx_cb_handler, NULL));
  // Manually call the callback function with LEFT_SIGNAL voltage
  control_stalk_callback(STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE, PERIODIC_READER_ID_0, NULL);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_CONTROL_STALK_EVENT_LEFT_SIGNAL,
                                   (uint16_t)STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(steering_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(STEERING_CAN_EVENT_TX, STEERING_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, count);
}

void test_control_stalk_right_signal_with_simultaneous_calls() {
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, prv_test_signal_rx_cb_handler, NULL));
  // Only a single event should be raised when there are multiple simulataneous calls
  // with slightly different voltage values
  control_stalk_callback(STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE, PERIODIC_READER_ID_0, NULL);
  control_stalk_callback(STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE + 5, PERIODIC_READER_ID_0,
                         NULL);
  control_stalk_callback(STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE - 5, PERIODIC_READER_ID_0,
                         NULL);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_CONTROL_STALK_EVENT_RIGHT_SIGNAL,
                                   (uint16_t)STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(steering_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(STEERING_CAN_EVENT_TX, STEERING_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(2, count);
}

void test_invalid_voltage() {
  control_stalk_callback(INVALID_VOLTAGE, PERIODIC_READER_ID_0, NULL);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
}

void teardown_test(void) {}
