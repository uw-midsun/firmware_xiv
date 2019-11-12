#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TEST_CAN_DEVICE_ID 0x1

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received a message!\n");
  return STATUS_CODE_OK;
}

void init_can(void) {
  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
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

int main() {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  init_can();
  LOG_DEBUG("Hello World!\n");

  Event e = { 0 };
  int i = 0;

  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
      LOG_DEBUG("Event Queue: empty%d!\r", i);
      i++;
      i %= 2;
    }
    can_process_event(&e);
  }

  return 0;
}
