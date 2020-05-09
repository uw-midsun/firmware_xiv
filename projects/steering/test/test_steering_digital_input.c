#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
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
#include "steering_digital_input.h"
#include "steering_events.h"
#include "test_helpers.h"
#include "unity.h"
#include "wait.h"

#define STEERING_CAN_DEVICE_ID 0x1

typedef enum {
  STEERING_CAN_EVENT_RX = 10,
  STEERING_CAN_EVENT_TX,
  STEERING_CAN_FAULT,
} SteeringCanEvent;

CanSettings can_settings = { .device_id = STEERING_CAN_DEVICE_ID,
                             .bitrate = CAN_HW_BITRATE_125KBPS,
                             .rx_event = STEERING_CAN_EVENT_RX,
                             .tx_event = STEERING_CAN_EVENT_TX,
                             .fault_event = STEERING_CAN_FAULT,
                             .tx = { GPIO_PORT_A, 12 },
                             .rx = { GPIO_PORT_A, 11 },
                             .loopback = true };

int count = 0;
static CanStorage s_can_storage;

StatusCode prv_test_horn_rx_cb_handler(const CanMessage *msg, void *context,
                                       CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(STEERING_INPUT_HORN_EVENT, msg->msg_id);
  count++;
  return STATUS_CODE_OK;
}

StatusCode prv_test_high_beam_rx_cb_handler(const CanMessage *msg, void *context,
                                            CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(STEERING_HIGH_BEAM_FORWARD_EVENT, msg->msg_id);
  count++;
  return STATUS_CODE_OK;
}

StatusCode prv_test_cc_toggle_rx_cb_handler(const CanMessage *msg, void *context,
                                            CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT, msg->msg_id);
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
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_HORN, prv_test_horn_rx_cb_handler, NULL);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_LIGHTS, prv_test_high_beam_rx_cb_handler, NULL);
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_HORN, prv_test_cc_toggle_rx_cb_handler, NULL);
}

void test_steering_digital_input_horn() {
  GpioAddress *horn_address = test_get_address(STEERING_INPUT_HORN_EVENT);
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(horn_address));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_INPUT_HORN_EVENT, (uint16_t)GPIO_STATE_LOW);
  // Should be empty after the event is popped off
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  MS_TEST_HELPER_CAN_RX(STEERING_CAN_EVENT_RX);
  printf("%d\n",e.data);
  TEST_ASSERT_OK(steering_can_process_event(&e));
  TEST_ASSERT_EQUAL(1, count);
}
/*
void test_steering_digital_input_high_beam_forward() {
  GpioAddress *high_beam_forward_address = test_get_address(STEERING_HIGH_BEAM_FORWARD_EVENT);
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(high_beam_forward_address));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_HIGH_BEAM_FORWARD_EVENT,
                                   (uint16_t)GPIO_STATE_LOW);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  MS_TEST_HELPER_CAN_RX(STEERING_CAN_EVENT_RX);
  printf("%d\n",e.id);
  TEST_ASSERT_OK(steering_can_process_event(&e));
  TEST_ASSERT_EQUAL(2, count);
}

void test_steering_digital_input_cc_toggle() {
  GpioAddress *cc_toggle_address = test_get_address(STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT);
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(cc_toggle_address));
  Event e = { 0 };
  MS_TEST_HELPER_ASSERT_NEXT_EVENT(e, (EventId)STEERING_INPUT_CC_TOGGLE_PRESSED_EVENT,
                                   (uint16_t)GPIO_STATE_LOW);
  MS_TEST_HELPER_ASSERT_NO_EVENT_RAISED();
  MS_TEST_HELPER_CAN_RX(STEERING_CAN_EVENT_RX);
  TEST_ASSERT_OK(steering_can_process_event(&e));
  TEST_ASSERT_EQUAL(3, count);
}
*/
void test_invalid_can_message() {
  // provide an invalid id
  Event e = { .id = 16, .data = 0 };
  TEST_ASSERT_NOT_OK(steering_can_process_event(&e));
}

void teardown_test(void) {}
