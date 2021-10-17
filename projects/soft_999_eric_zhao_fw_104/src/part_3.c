#include "can.h"
#include "can_msg.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_DEVICE_ID 0x1

#define DELAY_TIME_A_MS 2000
#define DELAY_TIME_B_MS 1000

#define RANDOM_16_BIT_A 123
#define RANDOM_16_BIT_B 321

typedef enum {
  CAN_EVENT_RX = 0,
  CAN_EVENT_TX,
  CAN_EVENT_FAULT,
} CanEvent;

static CanStorage s_can_storage = { 0 };

static StatusCode prv_ack_callback_status(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                          uint16_t num_remaining, void *context) {
  // Is this called right when an ack is detected, before the callback is called?
  LOG_DEBUG("ACK RECIEVED\n");

  return STATUS_CODE_OK;
}

static void prv_can_transmit_a(SoftTimerId timer_id, void *context) {
  CanMessage message = {
    .source_id = CAN_DEVICE_ID,
    .msg_id = 0xA,
    .data_u16 = { RANDOM_16_BIT_A },
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 2,
  };

  CanAckRequest ack_req = {
    .callback = prv_ack_callback_status,
    .context = NULL,
    .expected_bitset = 1 << CAN_DEVICE_ID,
  };

  can_transmit(&message, &ack_req);

  soft_timer_start_millis(DELAY_TIME_A_MS, prv_can_transmit_a, NULL, NULL);
}

static StatusCode prv_rx_a_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  // What happens if no acknowledgement is detected? Does this callback still run?
  *ack_reply = CAN_ACK_STATUS_OK;
  // Also, what happens if I set the *ack_reply to some other value?

  LOG_DEBUG("Received an A message!\n");

  uint16_t data = 0;
  for (uint8_t i = 0; i < msg->dlc; i++) {
    data += ((msg->data >> (i * 8)) & 0xFF) << (i * 8);
  }
  LOG_DEBUG("Data: %d\n\n", data);

  return STATUS_CODE_OK;
}

static void prv_can_transmit_b(SoftTimerId timer_id, void *context) {
  CanMessage message = {
    .source_id = CAN_DEVICE_ID,
    .msg_id = 0xB,
    .data_u16 = { RANDOM_16_BIT_B, 0, 0, 0 },
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 2,
  };
  can_transmit(&message, NULL);

  soft_timer_start_millis(DELAY_TIME_B_MS, prv_can_transmit_b, NULL, NULL);
}

static StatusCode prv_rx_b_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received a B message!\n");
  uint16_t data = 0;
  for (uint8_t i = 0; i < msg->dlc; i++) {
    data += ((msg->data >> (i * 8)) & 0xFF) << (i * 8);
  }
  LOG_DEBUG("Data: %d\n\n", data);
  return STATUS_CODE_OK;
}

int main(void) {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  const CanSettings can_settings = {
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

  can_register_rx_handler(0xA, prv_rx_a_callback, NULL);
  can_register_rx_handler(0xB, prv_rx_b_callback, NULL);

  soft_timer_start_millis(DELAY_TIME_A_MS, prv_can_transmit_a, NULL, NULL);
  soft_timer_start_millis(DELAY_TIME_B_MS, prv_can_transmit_b, NULL, NULL);

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  return 0;
}
