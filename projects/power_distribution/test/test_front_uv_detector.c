
#include "can_transmit.h"
#include "front_uv_detector.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "ms_test_helper_can.h"
#include "ms_test_helpers.h"
#include "pd_events.h"
#include "pin_defs.h"
#include "status.h"

#include "log.h"

static GpioAddress s_uv_comp_pin_address = FRONT_UV_COMPARATOR_PIN;

static volatile bool s_interrupt_ran = false;

static CanStorage s_can_storage;

typedef enum {
  TEST_CAN_EVENT_TX = 0,
  TEST_CAN_EVENT_RX,
  TEST_CAN_EVENT_FAULT,
  NUM_TEST_CAN_EVENTS,
} TestCanEvent;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  s_interrupt_ran = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  initialize_can_and_dependencies(&s_can_storage, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT,
                                  TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX, TEST_CAN_EVENT_FAULT);

  TEST_ASSERT_OK(front_uv_detector_init(&s_uv_comp_pin_address));

  s_interrupt_ran = false;
}

void teardown_test(void) {}

// test that CAN message gets sent after lockout
void test_uv_front_detector_notification(void) {
  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_uv_comp_pin_address));

  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_UV_CUTOFF_NOTIFICATION, prv_rx_callback, NULL));
  MS_TEST_HELPER_CAN_TX_RX(TEST_CAN_EVENT_TX, TEST_CAN_EVENT_RX);
  TEST_ASSERT_TRUE(s_interrupt_ran);
}
