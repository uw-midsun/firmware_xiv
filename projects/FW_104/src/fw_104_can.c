#include "fw_104_can.h"
#include "can.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_SEND_TIME_MS 1000

static StatusCode prv_can_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  // Taken from smoke_can
  LOG_DEBUG("Received a message!\n");
  printf("Data:\n\t");
  for (uint8_t i = 0; i < msg->dlc; i++) {
    uint8_t byte = (msg->data >> (i * 8)) & 0xFF;
    printf("0x%x ", byte);
  }
  printf("\n");
  return STATUS_CODE_OK;
}

static void prv_can_transmit(SoftTimerId timer_id, void *context) {
  uint8_t messageData = 5;
  CanMessage message = {
    .source_id = 0x1, .msg_id = 0x1, .data_u8 = messageData, .type = CAN_MSG_TYPE_DATA, .dlc = 8
  };
  can_transmit(&message, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, prv_can_transmit, NULL, NULL);
}

static StatusCode prv_write_A_message() {
  interrupt_init();
  soft_timer_init();

  CanStorage storage;

  CanSettings settings = {
    .device_id = 0xA,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    // Not really sure what to put here, following smoke_can
    .rx_event = 0,
    .tx_event = 1,
    .fault_event = 2,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&storage, &settings);

  can_register_rx_default_handler(prv_can_callback, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit, NULL, NULL);
}

static StatusCode prv_write_B_message() {
  interrupt_init();
  soft_timer_init();

  CanStorage storage;

  CanSettings settings = {
    .device_id = 0xB,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    // Not really sure what to put here, following smoke_can
    .rx_event = 0,
    .tx_event = 1,
    .fault_event = 2,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&storage, &settings);

  can_register_rx_default_handler(prv_can_callback, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit, NULL, NULL);
}
