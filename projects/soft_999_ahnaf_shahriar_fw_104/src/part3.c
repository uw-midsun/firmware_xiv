#include "can.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define DELAY_MS 1000

#define CAN_DEVICE_ID 0x1

typedef enum {
  CAN_RX = 0,
  CAN_TX,
  CAN_FAULT,
  NUM_CAN_EVENT,
} CanEvent;

// useless parameters needed for the callback function to match Can request
static StatusCode prv_ack_req_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                       uint16_t num_remaining, void *context) {
  if (status == CAN_ACK_STATUS_OK) {
    LOG_DEBUG("(%x) Ack recieved\n", msg_id);
  } else {
    LOG_DEBUG("(%x) No Ack\n", msg_id);
  }
  return STATUS_CODE_OK;
}

static void prv_can_transmit(SoftTimerId id, void *context) {
  CanMessage *msg = context;
  if (msg->msg_id == 0xA) {
    CanAckRequest ack_req = {
      .callback = prv_ack_req_callback,
      .context = NULL,
      .expected_bitset = 0b000000010,  // ack is the second last bit
    };
    can_transmit(msg, &ack_req);
  } else {
    can_transmit(msg, NULL);
  }
  LOG_DEBUG("Sent: (%x)\n", msg->msg_id);

  soft_timer_start_millis(DELAY_MS, prv_can_transmit, context, NULL);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Recieved: (%x) data: 0x", msg->msg_id);
  for (uint8_t i = 0; i < msg->dlc; i++) {
    uint8_t byte = (msg->data >> (i * 8)) & 0xFF;
    printf("%x", byte);
  }
  printf("\n");
  if (msg->msg_id == 0xA) {
    // ack message
    *ack_reply = CAN_ACK_STATUS_OK;
  }
  return STATUS_CODE_OK;
}

int main(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanStorage can_storage = { 0 };

  const CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .rx_event = CAN_RX,
    .tx_event = CAN_TX,
    .fault_event = CAN_FAULT,
    .loopback = false,
  };

  CanMessage message_a = {
    .source_id = CAN_DEVICE_ID,
    .msg_id = 0xA,
    .data_u16 = { 0x0000 },
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 2,
  };

  CanMessage message_b = {
    .source_id = CAN_DEVICE_ID,
    .msg_id = 0xB,
    .data_u16 = { 0x1111 },
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 2,
  };

  can_init(&can_storage, &can_settings);

  can_register_rx_default_handler(prv_rx_callback, NULL);

  soft_timer_start_millis(DELAY_MS, prv_can_transmit, &message_a, NULL);
  soft_timer_start_millis(DELAY_MS, prv_can_transmit, &message_b, NULL);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  return 0;
}