/*
Write a function that periodically sends a CAN message with id 0xA and a random uint16_t as the
body, and requests an ACK with an OK status. Write another function that periodically sends a CAN
message with id 0xB and a different random uint16_t as the body.

Then, register callbacks for both that print the data, but only ACK the 0xA message.

Run the program in two terminals at the same time, and send a screenshot of the output to your lead.
*/

#include <string.h>

#include "can.h"
#include "can_msg.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_DEVICE_A_ID 0xA
#define CAN_MSG_ID 0xA

#define SEND_TIME_MS 1000

#define DATA \
  { 0xfef }

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
  NUM_CAN_EVENTS,
} CanEvent;

static CanStorage s_can_storage;

void init_can(void) {
  CanSettings can_settings = {
    .device_id = CAN_DEVICE_A_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = CAN_EVENT_RX,
    .tx_event = CAN_EVENT_TX,
    .fault_event = CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&s_can_storage, &can_settings);
}

static void prv_can_transmit_a(SoftTimerId timer_id, void *context) {
  CanMessage can_message = {
    .source_id = CAN_DEVICE_A_ID,
    .msg_id = CAN_MSG_ID,
    .data_u16 = DATA,
    .type = CAN_MSG_TYPE_ACK,
    .dlc = 16,
  };

  can_transmit(&can_message, NULL);
}

void prv_message_a_handler(void) {
  soft_timer_start(SEND_TIME_MS, prv_can_transmit_a, NULL, NULL);
}

int main(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  init_can();

  prv_message_a_handler();
}
