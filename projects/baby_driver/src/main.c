
// This project will receive a CAN message
// on the stm32 andperform specific callback
// based on the message

// To run this project, simply run
// make babydriver
// and that should program to the stm32
// To send a CAN message write
// can_message.send_message(<data>) to send can message

#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"

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

// define callback functions here

int main() {
  LOG_DEBUG("Welcome to BabyDriver!\n");
  gpio_init();
  event_queue_init();
  interrupt_init();

  can_init(&s_can_storage, &s_can_settings);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }

  return 0;
}
