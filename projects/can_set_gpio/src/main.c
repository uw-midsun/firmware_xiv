#include <string.h>

#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TEST_CAN_DEVICE_ID 0x1

typedef enum {
  TEST_CAN_EVENT_RX = 0,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

void init_can(void) {
  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  can_register_rx_default_handler(prv_rx_callback, NULL);
}

static const GpioAddress gpio = { .port = GPIO_PORT_B, .pin = 5 };

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  gpio_set_state(&gpio, GPIO_STATE_HIGH);
  return STATUS_CODE_OK;
}

int main() {
  LOG_DEBUG("Welcome to CAN Set GPIO!\n");
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  GpioSettings led_settings = { .direction = GPIO_DIR_OUT,
                                .state = GPIO_STATE_HIGH,
                                .alt_function = GPIO_ALTFN_NONE,
                                .resistor = GPIO_RES_NONE };
  gpio_init_pin(&gpio, &led_settings);

  init_can();

  Event e = { 0 };
  int i = 0;

  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }

  return 0;
}
