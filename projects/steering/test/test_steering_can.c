#include "can.h"
#include "can_msg_defs.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt_def.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "soft_timer.h"
#include "status.h"
#include "stdint.h"
#include "steering_can.h"
#include "steering_digital_input.h"
#include "steering_events.h"
#include "test_helpers.h"

static CanStorage s_can_storage;

static CanSettings can_settings = { .device_id = SYSTEM_CAN_DEVICE_STEERING,
                                    .bitrate = CAN_HW_BITRATE_500KBPS,
                                    .rx_event = STEERING_CAN_EVENT_RX,
                                    .tx_event = STEERING_CAN_EVENT_TX,
                                    .fault_event = STEERING_CAN_FAULT,
                                    .tx = { GPIO_PORT_A, 12 },
                                    .rx = { GPIO_PORT_A, 11 },
                                    .loopback = true };

static uint8_t s_msg_rx_counter = 0;

static StatusCode prv_test_regen_toggle_rx_cb_handler(const CanMessage *msg, void *context,
                                                      CanAckStatus *ack_reply) {
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_REGEN_BRAKING_TOGGLE_REQUEST, msg->msg_id);
  s_msg_rx_counter++;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, can_settings.device_id, can_settings.tx_event,
                                  can_settings.rx_event, can_settings.fault_event);

  s_msg_rx_counter = 0;
}

void teardown_test(void) {}

void test_steering_can_process_event_regen_brake_toggling(void) {
  TEST_ASSERT_OK(can_register_rx_handler(SYSTEM_CAN_MESSAGE_REGEN_BRAKING_TOGGLE_REQUEST,
                                         prv_test_regen_toggle_rx_cb_handler, NULL));
  Event e = { STEERING_DIGITAL_INPUT_REGEN_BRAKE_TOGGLE, 0 };
  TEST_ASSERT_OK(steering_can_process_event(&e));
  MS_TEST_HELPER_CAN_TX_RX(STEERING_CAN_EVENT_TX, STEERING_CAN_EVENT_RX);
  TEST_ASSERT_EQUAL(1, s_msg_rx_counter);
}
