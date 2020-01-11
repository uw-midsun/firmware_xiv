#include <string.h>
#include "can.h"
#include "event_queue.h"
#include "events.h"
#include "log.h"

// should get the device id another way

CanMessage messages[NUM_BRAKE_CAN_EVENTS];

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  //again purely for testing
  event_raise(msg->data, 1);
  ///////////
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


StatusCode pedal_can_init(CanStorage *can_storage, CanSettings *can_settings) {
  CanMessage mes = {
    .msg_id = can_storage->device_id,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 1,
    .data = 1,
  };
  messages[CAN_BRAKE_PRESSED] = mes;
  mes.data = 2;
  messages[CAN_BRAKE_RELEASED] = mes;
  can_init(can_storage, can_settings);
  //must initialize a receive handler
  //temp purely for testing purposes
  can_register_rx_default_handler(prv_rx_callback, NULL);
  //////////////
  return STATUS_CODE_OK;
}

bool pedal_can_process_event(Event *e) {
  if (e->id < NUM_BRAKE_CAN_EVENTS) {
    can_transmit(&messages[e->id], NULL);
    LOG_DEBUG("Transmitted can message\n");
    return true;
  }
  return false;
}