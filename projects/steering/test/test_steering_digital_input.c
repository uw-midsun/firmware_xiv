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
#include "steering_digital_input.h"
#include "steering_events.h"
#include "test_helpers.h"

#define INVALID_STEERING_CAN_ID \
  { .id = 16, .data = 0 }

static CanStorage s_can_storage;

static CanSettings can_settings = { .device_id = SYSTEM_CAN_DEVICE_STEERING,
                                    .bitrate = CAN_HW_BITRATE_500KBPS,
                                    .rx_event = STEERING_CAN_EVENT_RX,
                                    .tx_event = STEERING_CAN_EVENT_TX,
                                    .fault_event = STEERING_CAN_FAULT,
                                    .tx = { GPIO_PORT_A, 12 },
                                    .rx = { GPIO_PORT_A, 11 },
                                    .loopback = true };

static int count = 0;

StatusCode prv_test_horn_rx_cb_handler(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_HORN, msg->msg_id);
  count++;
  return STATUS_CODE_OK;
}

StatusCode prv_test_high_beam_rx_cb_handler(const CanMessage *msg, void *context,
                                            CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_LIGHTS, msg->msg_id);
  count++;
  return STATUS_CODE_OK;
}

StatusCode prv_test_cc_toggle_rx_cb_handler(const CanMessage *msg, void *context,
                                            CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND, msg->msg_id);
  count++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  event_queue_init();
  gpio_it_init();
  soft_timer_init();
  TEST_ASSERT_OK(steering_digital_input_init());
  TEST_ASSERT_OK(can_init(&s_can_storage, &can_settings));
}

void test_steering_digital_input_horn() {
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_HORN, prv_test_horn_rx_cb_handler, NULL));
  GpioAddress horn_address = HORN_GPIO_ADDR;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&horn_address));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_INPUT_HORN_EVENT, (uint16_t)GPIO_STATE_LOW);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(steering_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(STEERING_CAN_EVENT_TX, STEERING_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, count);
}

void test_steering_digital_input_drl_1() {
  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, prv_test_high_beam_rx_cb_handler, NULL));
  GpioAddress drl_1_address = DRL_1_GPIO_ADDR;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&drl_1_address));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_DRL_1_EVENT, (uint16_t)GPIO_STATE_LOW);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(steering_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(STEERING_CAN_EVENT_TX, STEERING_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(2, count);
}

void test_steering_digital_input_cc_toggle() {
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_CRUISE_CONTROL_COMMAND,
                                         prv_test_cc_toggle_rx_cb_handler, NULL));
  GpioAddress cc_toggle_address = CC_TOGGLE_GPIO_ADDR;
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&cc_toggle_address));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_DIGITAL_INPUT_CC_TOGGLE_PRESSED_EVENT,
                                   (uint16_t)GPIO_STATE_LOW);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  TEST_ASSERT_OK(steering_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(STEERING_CAN_EVENT_TX, STEERING_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(3, count);
}

void test_invalid_can_message() {
  Event e = INVALID_STEERING_CAN_ID;
  TEST_ASSERT_NOT_OK(steering_can_process_event(&e));
}

void teardown_test(void) {}
