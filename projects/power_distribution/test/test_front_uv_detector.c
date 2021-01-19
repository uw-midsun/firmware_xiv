#include "can_transmit.h"
#include "front_uv_detector.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "ms_test_helpers.h"
#include "pd_events.h"

static GpioAddress s_uv_comp_pin_address = { .port = GPIO_PORT_B, .pin = 0 };

static volatile bool s_interrupt_ran = false;

static CanStorage s_can_storage;

static void prv_initialize_can(SystemCanDevice can_device) {
  CanSettings can_settings = {
    .device_id = can_device,
    .loopback = true,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = POWER_DISTRIBUTION_CAN_EVENT_RX,
    .tx_event = POWER_DISTRIBUTION_CAN_EVENT_TX,
    .fault_event = POWER_DISTRIBUTION_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
  };
  can_init(&s_can_storage, &can_settings);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  s_interrupt_ran = true;
  return STATUS_CODE_OK;
}

void setup_test(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();
  front_uv_detector_init();

  s_interrupt_ran = false;
}

void teardown_test(void) {}

// test that CAN message gets sent after lockout
void test_uv_front_detector_notification() {
  prv_initialize_can(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);

  TEST_ASSERT_OK(gpio_it_trigger_interrupt(&s_uv_comp_pin_address));

  TEST_ASSERT_OK(
      can_register_rx_handler(SYSTEM_CAN_MESSAGE_UV_CUTOFF_NOTIFICATION, prv_rx_callback, NULL));
  MS_TEST_HELPER_CAN_TX_RX(POWER_DISTRIBUTION_CAN_EVENT_TX, POWER_DISTRIBUTION_CAN_EVENT_RX);
  TEST_ASSERT_TRUE(s_interrupt_ran);
}
