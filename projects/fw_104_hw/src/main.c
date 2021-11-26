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

#define SEND_TIME_MS 1000

#define CAN_DEVICE_ID 0x1
#define CAN_MSG_ID 0xA

#define CAN_MSG_DATA \
  { 0xfef }

#define CAN_MSG_SIZE 2

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
  LOG_DEBUG("ACK received\n");
  return STATUS_CODE_OK;
}

static void prv_can_transmit(SoftTimerId timer_id, void *context) {
  CanMessage can_message = {
    .source_id = CAN_DEVICE_ID,
    .msg_id = CAN_MSG_ID,
    .data_u16 = CAN_MSG_DATA,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = CAN_MSG_SIZE,
  };

  CanAckRequest ack_request = {
    .callback = prv_ack_handler,
    .context = NULL,
    .expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_DEVICE_ID),
  };

  can_transmit(&can_message, &ack_request);
  soft_timer_start_millis(SEND_TIME_MS, prv_can_transmit, NULL, NULL);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received a message!\n");
  printf("Data:\n\t");
  for (uint8_t i = 0; i < msg->dlc; i++) {
    uint8_t byte = (msg->data >> (i * 8)) & 0xFF;
    printf("0x%x ", byte);
  }
  printf("\n");
  return STATUS_CODE_OK;
}

int main(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  LOG_DEBUG("Welcome to CAN\n");

  init_can();
  StatusCode status = can_register_rx_handler(CAN_MSG_ID, prv_rx_callback, NULL);
  if (status == STATUS_CODE_OK) {
    LOG_DEBUG("Registered successfully\n");
  }

  soft_timer_start_millis(SEND_TIME_MS, prv_can_transmit, NULL, NULL);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      // rx_handler isn't being called even though can messages are being sent
      can_process_event(&e);
    }
  }

  return 0;
}
