#include "unity.h"
#include "can.h"
#include "status.h"
#include "exported_enums.h"
#include "can_transmit.h"
#include "can_msg_defs.h"
#include "ms_test_helpers.h"
#include "log.h"
#include "event_queue.h"
#include "centre_console_events.h"
#include "power_on_sequence_fsm.h"
#include "interrupt.h"
#include "test_helpers.h"
#include "gpio.h"
#include <string.h>

typedef struct TestCanMessageAssertion {
  CanMessageId message_id;
  uint64_t data;
} TestCanMessageAssertion;

static PowerOnSequenceFsmStorage s_sequence_storage;

static CanStorage s_can_storage;

const CanSettings s_can_settings = {
  // clang-format on
  .device_id = SYSTEM_CAN_DEVICE_CENTRE_CONSOLE,
  .loopback = true,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx = { .port = GPIO_PORT_A, .pin = 11 },
  .tx = { .port = GPIO_PORT_A, .pin = 12 },
  .rx_event = CENTRE_CONSOLE_EVENT_CAN_RX,
  .tx_event = CENTRE_CONSOLE_EVENT_CAN_TX,
  .fault_event = CENTRE_CONSOLE_EVENT_CAN_FAULT
};

static TestCanMessageAssertion s_can_message_assertion;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  s_can_message_assertion.message_id = msg->msg_id;
  s_can_message_assertion.data = msg->data;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  TEST_ASSERT_OK(can_init(&s_can_storage, &s_can_settings));
  can_register_rx_handler(SYSTEM_CAN_MESSAGE_FRONT_POWER, 
    prv_rx_callback, 
    NULL
  );
  memset(&s_sequence_storage, 0, sizeof(s_sequence_storage));
  memset(&s_can_message_assertion, 0, sizeof(s_can_message_assertion));

  power_on_sequence_fsm_init(&s_sequence_storage);
}

void teardown_test(void) {}

void test_sequence_starts_by_turning_on_driver_display(void) {
  // given
  Event e = { .id = POWER_ON_SEQUENCE_EVENT_BEGIN, .data = 0 };
  // when
  power_on_sequence_fsm_process_event(&s_sequence_storage, &e);
  // then
  MS_TEST_HELPER_CAN_TX_RX(CENTRE_CONSOLE_EVENT_CAN_TX, CENTRE_CONSOLE_EVENT_CAN_RX);
  TEST_ASSERT_EQUAL(SYSTEM_CAN_MESSAGE_FRONT_POWER, s_can_message_assertion.message_id);
  TEST_ASSERT_EQUAL(1 << EE_FRONT_POWER_DISTRIBUTION_OUTPUT_DRIVER_DISPLAY, s_can_message_assertion.data);
}



