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

#define CAN_DEVICE_ID 0xA
#define CAN_MSG_ID 0xA

#define SEND_TIME_MS 1

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
    .device_id = CAN_DEVICE_ID,
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

static StatusCode prv_ack_handler(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                  uint16_t num_remaining, void *context) {
  if (status == CAN_ACK_STATUS_OK) {
    LOG_DEBUG("Yo, I'm acknowledged and I'm status code okay!");
  }
  return STATUS_CODE_OK;
}

static void prv_can_transmit(SoftTimerId timer_id, void *context) {
  LOG_DEBUG("Iteration....");
  CanMessage can_message = {
    .source_id = CAN_DEVICE_ID,
    .msg_id = CAN_MSG_ID,
    .data_u16 = DATA,
    .type = CAN_MSG_TYPE_ACK,
    .dlc = 16,
  };

  CanAckRequest ack_request = {
    .callback = prv_ack_handler,
    .context = NULL,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_ID),
  };

  can_transmit(&can_message, &ack_request);
}

int main(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  init_can();

  soft_timer_start(SEND_TIME_MS, prv_can_transmit, NULL, NULL);

  Event e = { 0 };
  while (true) {
  }
}
