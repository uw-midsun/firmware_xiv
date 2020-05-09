#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "adc.h"
#include "adc_periodic_reader.h"
#include "can.h"
#include "can_msg_defs.h"
#include "delay.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "misc.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "steering_can.h"
#include "steering_control_stalk.h"
#include "steering_digital_input.h"
#include "steering_events.h"
#include "test_helpers.h"
#include "wait.h"

#define STEERING_CAN_DEVICE_ID 0x1
#define STEERING_CONTROL_STALK_LEFT_SIGNAL_VOLTAGE 1000
#define STEERING_CONTROL_STALK_RIGHT_SIGNAL_VOLTAGE 2000
#define STEERING_CC_INCREASE_SPEED_VOLTAGE 3000
#define STEERING_CC_DECREASE_SPEED_VOLTAGE 4000
#define STEERING_CC_BRAKE_PRESSED_VOLTAGE 5000
#define TOL 100

typedef enum {
  STEERING_CAN_EVENT_RX = 10,
  STEERING_CAN_EVENT_TX,
  STEERING_CAN_FAULT,
} SteeringCanEvent;

CanSettings can_settings = {
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
int count = 0;

StatusCode prv_test_cc_increase_rx_cb_handler(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND, msg->msg_id);
  count++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  adc_init(ADC_MODE_SINGLE);
  TEST_ASSERT_OK(gpio_init());
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  TEST_ASSERT_OK(steering_digital_input_init());
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
  TEST_ASSERT_OK(adc_periodic_reader_init());
  TEST_ASSERT_OK(control_stalk_init());
}

void test_control_stalk_cc_increse_speed() {
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND, prv_test_cc_increase_rx_cb_handler, NULL));
  //Manually call the callback function with the CC_INCREASE_SPEED voltage
  //to raise an event
  control_stalk_callback(STEERING_CC_INCREASE_SPEED_VOLTAGE, PERIODIC_READER_ID_0, NULL);
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_CC_EVENT_INCREASE_SPEED,STEERING_CC_INCREASE_SPEED_VOLTAGE);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(steering_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(STEERING_CAN_EVENT_TX, STEERING_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, count);
}

void teardown_test(void) {}
