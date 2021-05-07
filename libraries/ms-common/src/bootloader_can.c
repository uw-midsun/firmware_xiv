#include "bootloader_can.h"

#include <stdint.h>
#include <string.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "status.h"

#define CLIENT_SCRIPT_CONTROLLER_BOARD_ID 0

static BootloaderCanCallback s_callback = NULL;
static void *s_context = NULL;

StatusCode bootloader_can_init(CanStorage *storage, CanSettings *settings) {
  settings->device_id = SYSTEM_CAN_DEVICE_BOOTLOADER;

  return can_init(storage, settings);
}

StatusCode bootloader_can_transmit(uint16_t board_id, uint8_t *data, size_t len,
                                   bool is_start_message) {
  if (len > 8) {
    return STATUS_CODE_INVALID_ARGS;
  }
  CanMessage message = { .source_id = SYSTEM_CAN_DEVICE_BOOTLOADER,
                         .msg_id = board_id,
                         .type = CAN_MSG_TYPE_DATA,
                         .dlc = len };
  // copy the message data over to the message to be transmitted
  memcpy(message.data_u8, data, len);
  // if the message is a start message, set the ACK bit to 1 by changing type to CAN_MSG_TYPE_ACK
  if (is_start_message) {
    message.type = CAN_MSG_TYPE_ACK;
  }
  return can_transmit(&message, NULL);
}

StatusCode bootloader_can_register_handler(BootloaderCanCallback callback, void *context) {
  if (callback == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_callback = callback;
  s_context = context;
  return STATUS_CODE_OK;
}

StatusCode bootloader_can_receive(CanMessage *msg) {
  if (msg->msg_id == CLIENT_SCRIPT_CONTROLLER_BOARD_ID && s_callback != NULL) {
    return s_callback(msg, s_context);
  }
  return STATUS_CODE_OK;
}
