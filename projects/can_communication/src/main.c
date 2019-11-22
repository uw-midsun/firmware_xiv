#include <string.h>
#include "can.h"
#include "event_queue.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define TEST_CAN_DEVICE_ID 0x1

typedef enum {
  TEST_CAN_EVENT_RX = 10,
  TEST_CAN_EVENT_TX,
  TEST_CAN_EVENT_FAULT,
} TestCanEvent;

static CanStorage s_can_storage;

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply);

void init_can(void) {
  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    .rx_event = TEST_CAN_EVENT_RX,
    .tx_event = TEST_CAN_EVENT_TX,
    .fault_event = TEST_CAN_EVENT_FAULT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  StatusCode ret = can_init(&s_can_storage, &can_settings);
  can_register_rx_default_handler(prv_rx_callback, NULL);
}

static const GpioAddress s_leds[] = {
  { .port = GPIO_PORT_B, .pin = 5 },   //
  { .port = GPIO_PORT_B, .pin = 4 },   //
  { .port = GPIO_PORT_B, .pin = 3 },   //
  { .port = GPIO_PORT_A, .pin = 15 },  //
};

static void prv_init_leds() {
  GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE
  };
  for (size_t i = 0; i < SIZEOF_ARRAY(s_leds); i++) {
    gpio_init_pin(&s_leds[i], &led_settings);
  }
}

static void prv_toggle_leds() {
  for (size_t i = 0; i < SIZEOF_ARRAY(s_leds); i++) {
    gpio_toggle_state(&s_leds[i]);
  }
}

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  prv_toggle_leds();
  LOG_DEBUG("Received a message!\n");
  char log_message[30];
  strcpy(log_message, "Data:\n\t");
  uint8_t i;
  for (i = 0; i < msg->dlc; i++) {
      uint8_t byte = 0;
      byte = msg->data >> (i * 8);
      char buff[5];
      sprintf(buff, "%x ", byte);
      strcat(log_message, buff);
  }
  strcat(log_message, "\n");
  LOG_DEBUG("%s", log_message);
  return STATUS_CODE_OK;
}

int main() {
  gpio_init();
  event_queue_init();
  interrupt_init();
  soft_timer_init();
  
  prv_init_leds();

  init_can();
  LOG_DEBUG("Welcome to CAN Demo!\n");

  Event e = { 0 };
  int i = 0;

  while (true) {
    while (event_process(&e) != STATUS_CODE_OK) {
    }
    can_process_event(&e);
  }

  return 0;
}
