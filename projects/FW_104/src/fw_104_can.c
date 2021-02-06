#include "fw_104_can.h"

#include "can.h"
#include "can_ack.h"
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

typedef enum {
  TEST_CAN_ACK_DEVICE_A = 0,
  TEST_CAN_ACK_DEVICE_B,
  TEST_CAN_ACK_DEVICE_C,
  TEST_CAN_ACK_DEVICE_UNRECOGNIZED
} TestCanAckDevice;

StatusCode ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                        uint16_t num_remaining, void *context) {
  LOG_DEBUG("ACK handled: status %d from %d (msg %d) (%d remaining)\n", status, device, msg_id,
            num_remaining);
  if (status == CAN_ACK_STATUS_UNKNOWN) {
    LOG_DEBUG("Returning unknown code\n");
    return status_code(STATUS_CODE_UNKNOWN);
  }

  return STATUS_CODE_OK;
}

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
    .source_id = 0x1, .msg_id = 0xA, .data_u8 = { messageData }, .type = CAN_MSG_TYPE_DATA, .dlc = 1
  };

  CanAckRequest ackRequest = { .callback = ack_callback,
                               .context = ((void *)0),
                               .expected_bitset = CAN_ACK_EXPECTED_DEVICES(TEST_CAN_ACK_DEVICE_A) };
  can_transmit(&message, &ackRequest);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_A, NULL, NULL);
}

void can_transmit_B(SoftTimerId timer_id, void *context) {
  uint8_t messageData = 5;
  CanMessage message = {
    .source_id = 0x1, .msg_id = 0xB, .data_u8 = { messageData }, .type = CAN_MSG_TYPE_DATA, .dlc = 1
  };
  can_transmit(&message, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_B, NULL, NULL);
}

void write_A_message(void) {
  CanStorage storage;

  CanSettings settings = {
    .device_id = 0x1,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    // Not really sure what to put here, following smoke_can
    .rx_event = FW_104_CAN_RX_EVENT,
    .tx_event = FW_104_CAN_TX_EVENT,
    .fault_event = FW_104_CAN_FAULT_EVENT,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&storage, &settings);

  can_register_rx_default_handler(can_callback, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_A, NULL, NULL);
}

void write_B_message(void) {
  CanStorage storage;

  CanSettings settings = {
    .device_id = 0x1,
    .bitrate = CAN_HW_BITRATE_500KBPS,
    // Not really sure what to put here, following smoke_can
    .rx_event = 0,
    .tx_event = 1,
    .fault_event = 2,
    .tx = { GPIO_PORT_A, 12 },
    .rx = { GPIO_PORT_A, 11 },
    .loopback = false,
  };

  can_init(&storage, &settings);

  can_register_rx_default_handler(can_callback, NULL);

  soft_timer_start_millis(CAN_SEND_TIME_MS, can_transmit_B, NULL, NULL);
}
