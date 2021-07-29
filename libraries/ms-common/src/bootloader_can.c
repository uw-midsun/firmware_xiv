#include "bootloader_can.h"

#include <stdint.h>
#include <string.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "status.h"

#define CLIENT_SCRIPT_CONTROLLER_BOARD_ID 0

static BootloaderCanCallback s_callback = NULL;
static uint16_t s_board_id;

StatusCode bootloader_can_init(CanStorage *storage, CanSettings *settings, uint16_t board_id) {
  settings->device_id = SYSTEM_CAN_DEVICE_BOOTLOADER;
  s_board_id = board_id;
  return can_init(storage, settings);
}

StatusCode bootloader_can_transmit(uint8_t *data, size_t len, bool is_start_message) {
  if (len > 8) {
    return STATUS_CODE_INVALID_ARGS;
  }
  CanMessage message = {
    .source_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
    .msg_id = s_board_id,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = len,
  };
  // copy the message data over to the message to be transmitted
  memcpy(message.data_u8, data, len);
  // if the message is a start message, set the ACK bit to 1 by changing type to CAN_MSG_TYPE_ACK
  if (is_start_message) {
    message.type = CAN_MSG_TYPE_ACK;
  }
  return can_transmit(&message, NULL);
}

StatusCode bootloader_can_register_handler(BootloaderCanCallback callback) {
  if (callback == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_callback = callback;
  return STATUS_CODE_OK;
}

StatusCode bootloader_can_receive(CanMessage *msg) {
  if (msg->msg_id == CLIENT_SCRIPT_CONTROLLER_BOARD_ID && s_callback != NULL) {
    if (msg->type == CAN_MSG_TYPE_ACK) {  // msg is a start msg
      return s_callback(msg->data_u8, msg->dlc, true);
    } else {  // (msg->msg_id == CAN_MSG_TYPE_DATA) msg is a data msg
      return s_callback(msg->data_u8, msg->dlc, false);
    }
  }
  return STATUS_CODE_OK;
}
