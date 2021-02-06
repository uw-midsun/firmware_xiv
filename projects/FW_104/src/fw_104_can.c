#include "fw_104_can.h"

#include "can.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define CAN_SEND_TIME_MS 1000

typedef enum {
  FW_104_CAN_RX_EVENT = 0,
  FW_104_CAN_TX_EVENT,
  FW_104_CAN_FAULT_EVENT,
  NUM_FW_104_CAN_EVENTS,
} Fw104CanEvent;

StatusCode can_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
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

void can_transmit_A(SoftTimerId timer_id, void *context) {
  uint8_t messageData = 5;
  CanMessage message = {
    .source_id = 0x1, .msg_id = 0x1, .data_u8 = { messageData }, .type = CAN_MSG_TYPE_DATA, .dlc = 8
  };
  can_transmit(&message, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_A, NULL, NULL);
}

void can_transmit_B(SoftTimerId timer_id, void *context) {
  uint8_t messageData = 5;
  CanMessage message = {
    .source_id = 0x1, .msg_id = 0x1, .data_u8 = { messageData }, .type = CAN_MSG_TYPE_DATA, .dlc = 8
  };
  can_transmit(&message, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_B, NULL, NULL);
}

void write_A_message(void) {
  CanStorage storage;

  CanSettings settings = {
    .device_id = 0xA,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    // Not really sure what to put here, following smoke_can
    .rx_event = FW_104_CAN_RX_EVENT,
    .tx_event = FW_104_CAN_TX_EVENT,
    .fault_event = FW_104_CAN_FAULT_EVENT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = true,
  };

  can_init(&storage, &settings);

  can_register_rx_default_handler(can_callback, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_A, NULL, NULL);

  // The can_callback did not trigger unless I put this event queue stuff... (Copied from
  // smoke_can)
  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }
}

void write_B_message(void) {
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

  can_register_rx_default_handler(can_callback, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_B, NULL, NULL);

  // The can_callback did not trigger unless I put this event queue stuff... (Copied from
  // smoke_can)
  Event e = { 0 };
  while (true) {
    while (event_process(&e) == STATUS_CODE_OK) {
      can_process_event(&e);
    }
  }
}
