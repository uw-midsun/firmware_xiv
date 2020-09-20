// this is a simple smoke test to make sure can works
// simply run the project and it should periodically send can messages and receive them

// Configurable items: bitrate, send time, data
#include <string.h>

#include "can.h"
#include "can_msg.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TEST_CAN_DEVICE_ID 0x1
#define TEST_CAN_MSG_ID 0x1
#define TEST_CAN_BITRATE CAN_HW_BITRATE_500KBPS

// this is how often the message will send
// modify if you want to send more or less
#define SMOKETEST_SEND_TIME_MS 10
// this is the data it'll send
// this is an array of exactly 8 uint8_ts
// change if you wish
#define DATA \
  { 0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10 }

typedef enum {
  TEST_CAN_EVENT_RX = 0,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;

static void prv_can_transmit(SoftTimerId timer_id, void *context) {
  CanMessage message = { .source_id = TEST_CAN_DEVICE_ID,
                         .msg_id = TEST_CAN_MSG_ID,
                         .data_u8 = DATA,
                         .type = CAN_MSG_TYPE_DATA,
                         .dlc = 8 };
  can_transmit(&message, NULL);

  // soft_timer_start_millis(SMOKETEST_SEND_TIME_MS, prv_can_transmit, NULL, NULL);
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

int main() {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();

  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .bitrate = TEST_CAN_BITRATE,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&s_can_storage, &can_settings);
  can_register_rx_default_handler(prv_rx_callback, NULL);

  LOG_DEBUG("Initializing smoke test can\n");

  soft_timer_start_millis(SMOKETEST_SEND_TIME_MS, prv_can_transmit, NULL, NULL);
  CanMessage message = { .source_id = TEST_CAN_DEVICE_ID,
                         .msg_id = TEST_CAN_MSG_ID,
                         .data_u8 = DATA,
                         .type = CAN_MSG_TYPE_DATA,
                         .dlc = 8 };
  for (int i = 0; i < 10 i++) {
    can_transmit(&message, NULL);
  }

  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }

  return 0;
}
