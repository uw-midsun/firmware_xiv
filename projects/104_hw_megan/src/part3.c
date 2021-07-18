#include "can.h"
#include "can_ack.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define MSG_ID_A 0xA
#define MSG_ID_B 0xB
#define CAN_DEVICE_ID 0x1
#define SEND_TIME 1000

typedef enum {
  CAN_ACK_DEVICE_A = 0,
  CAN_ACL_DEVICE_B,
} Can_Ack_Devices;

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
} CanEvent;

static CanStorage s_can_storage = { 0 };

static StatusCode prv_ack_request(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                  uint16_t num_remaining, void *context) {
  if (status == CAN_ACK_STATUS_OK)
    LOG_DEBUG("acked %d\n", msg_id);
  else
    LOG_DEBUG("not acked %d\n", msg_id);

  return STATUS_CODE_OK;
}

static void prv_can_transmit_A(SoftTimerId timer_id, void *context) {
  CanMessage message_A = { .source_id = CAN_DEVICE_ID,
                           .msg_id = MSG_ID_A,
                           .data_u16 = 0xF8F3,
                           .type = CAN_MSG_TYPE_DATA,
                           .dlc = 2 };

  CanAckRequest ack_request = { .callback = prv_ack_request,
                                .context = NULL,
                                .expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_ACK_DEVICE_A) };

  can_transmit(&message_A, &ack_request);

  soft_timer_start_millis(SEND_TIME, prv_can_transmit_A, NULL, NULL);
}

static void prv_can_transmit_B(SoftTimerId timer_id, void *context) {
  CanMessage message_B = { .source_id = CAN_DEVICE_ID,
                           .msg_id = MSG_ID_B,
                           .data_u16 = 0x83DF,
                           .type = CAN_MSG_TYPE_DATA,
                           .dlc = 2 };

  can_transmit(&message_B, NULL);

  soft_timer_start_millis(SEND_TIME, prv_can_transmit_B, NULL, NULL);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  printf("Data: \n\t");
  uint8_t i;
  for (i = 0; i < msg->dlc; i++) {
    uint8_t byte = (msg->data >> (i * 8) & 0xFF);
    printf("%x", byte);
  }
  printf("\n");

  return STATUS_CODE_OK;
}

int main() {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

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
  can_register_rx_handler(MSG_ID_A, prv_rx_callback, NULL);
  can_register_rx_handler(MSG_ID_B, prv_rx_callback, NULL);

  soft_timer_start_millis(SEND_TIME, prv_can_transmit_A, NULL, NULL);
  soft_timer_start_millis(SEND_TIME, prv_can_transmit_B, NULL, NULL);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  return 0;
}
