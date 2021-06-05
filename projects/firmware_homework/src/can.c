#include "can_104.h"

#include "can.h"
#include "can_ack.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

typedef enum {
  CAN_ACK_DEVICE_A = 0,
  CAN_ACK_DEVICE_B,
} CAN_ACK_Devices;

#define CAN_SOURCE_ID 0x1
#define CAN_A_MESSAGE_ID 0XA
#define CAN_B_MESSAGE_ID 0xB
#define CAN_PERIOD_MS 1000

StatusCode ACK_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                        uint16_t remaining, void *context) {
  LOG_DEBUG("ACK Callback: status %d from %d (msg %d) (%d remaining)\n", status, device, msg_id,
            remaining);
  if (status != CAN_ACK_STATUS_OK) {
    return status_msg(STATUS_CODE_UNKNOWN, "Error message");
  }
  return STATUS_CODE_OK;
}

StatusCode CAN_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  // Modified from smoke CAN, I modified it for uint16's (not 100% sure if this is right)
  printf("Data:\n\t");
  for (uint16_t i = 0; i < msg->dlc; i++) {
    uint16_t byte = (msg->data >> (i * 16)) & 0xFFFF;
    printf("0x%x ", byte);
  }
  printf("\n");
  return STATUS_CODE_OK;
}

void CAN_A_send(SoftTimerId timer_id, void *context) {
  // CAN message, DLC is 2 since we are sending two bytes (uint16_t)
  uint16_t can_a_data = 7;
  CanMessage message_A = { .source_id = CAN_SOURCE_ID,
                           .msg_id = CAN_A_MESSAGE_ID,
                           .data_u16 = { can_a_data },
                           .type = CAN_MSG_TYPE_DATA,
                           .dlc = 2 };

  // ACK request message
  CanAckRequest ack_request = { .callback = ACK_callback,
                                .context = NULL,
                                .expected_bitset = CAN_ACK_EXPECTED_DEVICES(CAN_ACK_DEVICE_A) };

  can_transmit(&message_A, &ack_request);
  // Periodically send the CAN message
  soft_timer_start_millis(CAN_PERIOD_MS, CAN_A_send, NULL, NULL);
}

void CAN_B_send(SoftTimerId timer_id, void *context) {
  // CAN message, DLC is 2 since we are sending two bytes
  uint16_t can_b_data = 8;
  CanMessage message_B = { .source_id = CAN_SOURCE_ID,
                           .msg_id = CAN_B_MESSAGE_ID,
                           .data_u16 = { can_b_data },
                           .type = CAN_MSG_TYPE_DATA,
                           .dlc = 2 };

  // Do not do an acknowledge for CAN B
  can_transmit(&message_B, NULL);
  // Periodically send the CAN message
  soft_timer_start_millis(CAN_PERIOD_MS, CAN_B_send, NULL, NULL);
}

void write_CAN_messages(void) {
  // Start the CAN message sending
  soft_timer_start_millis(CAN_PERIOD_MS, CAN_A_send, NULL, NULL);
  soft_timer_start_millis(CAN_PERIOD_MS, CAN_B_send, NULL, NULL);
}
