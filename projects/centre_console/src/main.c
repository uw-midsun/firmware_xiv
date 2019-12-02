

#include "center_console.h"

#define TEST_CAN_DEVICE_ID 0x1

static CanStorage s_can_storage = { 0 };

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  // prv_toggle_leds();
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

void init_can(void) {
  CanSettings can_settings = {
    .device_id = TEST_CAN_DEVICE_ID,
    .bitrate = CAN_HW_BITRATE_125KBPS,
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

int main(void) {
  gpio_init();
  interrupt_init();
  soft_timer_init();
  event_queue_init();

  init_can();

  CenterConsoleStorage cc_storage = {
    .power_input =
        {
            .btn_addr = { .port = GPIO_PORT_B, .pin = 0 },
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_POWER,
        },
    .drive_input =
        {
            .btn_addr = { .port = GPIO_PORT_B, .pin = 1 },
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_DRIVE,
        },
    .neutral_input =
        {
            .btn_addr = { .port = GPIO_PORT_B, .pin = 2 },
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_NEUTRAL,
        },
    .reverse_input =
        {
            .btn_addr = { .port = GPIO_PORT_B, .pin = 0 },
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_REVERSE,
        },
    .hazards_input =
        {
            .btn_addr = { .port = GPIO_PORT_B, .pin = 0 },
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_HAZARDS,
        },
    .low_beam_input =
        {
            .btn_addr = { .port = GPIO_PORT_B, .pin = 0 },
            .can_event = EE_CENTER_CONSOLE_DIGITAL_INPUT_LOW_BEAM,
        },
  };

  initialize_center_console(&cc_storage);
  while (true) {
  }
  return 0;
}
