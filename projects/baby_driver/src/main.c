#include <string.h>

#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_DEVICE_ID 0x1

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
  NUM_CAN_EVENTS,
} CanEvent;

static CanStorage s_can_storage;
static CanSettings s_can_settings = {
  .device_id = CAN_DEVICE_ID,
  .bitrate = CAN_HW_BITRATE_500KBPS,
  .rx_event = CAN_EVENT_RX,
  .tx_event = CAN_EVENT_TX,
  .fault_event = CAN_EVENT_FAULT,
  .tx = { GPIO_PORT_A, 12 },
  .rx = { GPIO_PORT_A, 11 },
  .loopback = false,
};

static GpioSettings s_led_settings = { .direction = GPIO_DIR_OUT,
                                       .state = GPIO_STATE_HIGH,
                                       .alt_function = GPIO_ALTFN_NONE,
                                       .resistor = GPIO_RES_NONE };

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Receive CAN Message\n");
  return STATUS_CODE_OK;
}

int main() {
  LOG_DEBUG("Welcome to BabyDrier!\n");
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  can_init(&s_can_storage, &s_can_settings);
  can_register_rx_default_handler(prv_rx_callback, NULL);

  Event e = { 0 };
  int i = 0;

  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }

  return 0;
}
