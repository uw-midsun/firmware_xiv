// this is a simple smoke test to make sure can works

#include <string.h>

#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TEST_CAN_DEVICE_ID 0x1
#define TEST_CAN_BITRATE CAN_HW_BITRATE_500KBPS
// this is how often the message will send
// modify if you want to send more or less
#define SMOKETEST_WAIT_TIME_MS 1000

typedef enum {
  TEST_CAN_EVENT_RX = 0,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;

static void prv_can_transmit(SoftTimerId timer_id, void *context) {
  can_transmit();

  soft_timer_start_millis(SMOKETEST_WAIT_TIME_MS, prv_can_transmit, NULL, NULL);
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  LOG_DEBUG("Received a message!\n");
  char log_message[30];
  printf("Data:\n\t");
  uint8_t i;
  for (i = 0; i < msg->dlc; i++) {
    uint8_t byte = 0;
    byte = msg->data >> (i * 8);
    printf("%x ", byte);
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

  Event e = { 0 };

  soft_timer_start_millis(SMOKETEST_WAIT_TIME_MS, prv_can_transmit, NULL, NULL);

  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }

  return 0;
}
