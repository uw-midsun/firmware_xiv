#include "can.h"
#include "can_ack.h"
#include "can_msg.h"
#include "event_queue.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_DEVICE_ID 5
#define CAN_ID_A 0xA
#define CAN_ID_B 0xB
#define CAN_PERIOD_A_MS 1000
#define CAN_PERIOD_B_MS 2000

typedef enum {
  TEST_CAN_EVENT_RX = 0,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage = { 0 };

CanMessage messageA = {
  .source_id = CAN_DEVICE_ID,
  .msg_id = CAN_ID_A,
  .data = 4015,  // random uint16_t becomes uint64_t (not sure how to prevent)
  .type = CAN_MSG_TYPE_DATA,
  .dlc = 2,  // 2 bytes in a uint16_t?
};

CanMessage messageB = {
  .source_id = CAN_DEVICE_ID,
  .msg_id = CAN_ID_B,
  .data = 7777,  // random uint16_t becomes uint64_t (not sure how to prevent)
  .type = CAN_MSG_TYPE_DATA,
  .dlc = 2,  // 2 bytes in a uint16_t?
};

static StatusCode prv_rx_callbackA(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received message with id: 0x%x!\n", msg->msg_id);
  printf("Data:\n\t %lu", msg->data);
  // this loop gives me numbers I do not understand
  // for (uint8_t i = 0; i < msg->dlc; i++) {
  //   uint8_t byte = 0;
  //   byte = msg->data >> (i * 8);
  //   printf("0x%x ", byte);
  // }

  printf("\n");
  // ACK the message?

  return STATUS_CODE_OK;
}

static StatusCode prv_rx_callbackB(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received message with id: 0x%x!\n", msg->msg_id);
  printf("Data:\n\t %lu", msg->data);
  // this loop gives me numbers I do not understand
  // for (uint8_t i = 0; i < msg->dlc; i++) {
  //   uint8_t byte = 0;
  //   byte = msg->data >> (i * 8);
  //   printf("0x%x ", byte);
  // }
  printf("\n");

  return STATUS_CODE_OK;
}

void init_can(void) {
  LOG_DEBUG("Initialize Can\n");
  CanSettings can_settings = {
    .device_id = CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };
  can_init(&s_can_storage, &can_settings);
  can_register_rx_handler(CAN_ID_A, prv_rx_callbackA, NULL);
  can_register_rx_handler(CAN_ID_B, prv_rx_callbackB, NULL);
}

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  CanMessage *storage = context;

  // log output
  LOG_DEBUG("Transmit message: 0x%x!\n", storage->msg_id);
  if (storage->msg_id == CAN_ID_A) {
    can_transmit(storage, CAN_ACK_STATUS_OK);
    soft_timer_start_millis(CAN_PERIOD_A_MS, prv_timer_callback, storage, NULL);
  } else {
    can_transmit(storage, NULL);
    soft_timer_start_millis(CAN_PERIOD_B_MS, prv_timer_callback, storage, NULL);
  }
}

int main() {
  interrupt_init();
  soft_timer_init();
  init_can();
  event_queue_init();

  soft_timer_start_millis(CAN_PERIOD_A_MS, prv_timer_callback, &messageA, NULL);
  soft_timer_start_millis(CAN_PERIOD_B_MS, prv_timer_callback, &messageB, NULL);
  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }
  return 0;
}