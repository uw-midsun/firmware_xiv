#include <string.h>
#include "can.h"
#include "event_queue.h"
#include "log.h"
#include "pedal_events.h"

// just for testing
CanMessage messages[NUM_PEDAL_CAN_EVENTS];
// just for testing
static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  // again purely for testing
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
  //////////////just for testing
  CanMessage mes = {
    .msg_id = can_storage->device_id,
    .type = CAN_MSG_TYPE_DATA,
    .dlc = 0,
    .data = 0,
  };
  messages[PEDAL_CAN_EVENT_BRAKE_PRESSED] = mes;
  mes.dlc = 1;
  messages[PEDAL_CAN_EVENT_BRAKE_RELEASED] = mes;
  mes.dlc = 3;
  messages[PEDAL_EVENT_THROTTLE_READING] = mes;
  mes.dlc = 4;
  messages[PEDAL_THROTTLE_EVENT_DATA] = mes;
  mes.dlc = 5;
  messages[PEDAL_THROTTLE_EVENT_FAULT] = mes;
  mes.dlc = 6;
  messages[PEDAL_EVENT_THROTTLE_ENABLE] = mes;
  mes.dlc = 7;
  messages[PEDAL_EVENT_THROTTLE_DISABLE] = mes;
  mes.dlc = 8;
  messages[PEDAL_BRAKE_MONITOR_EVENT_FAULT] = mes;
  mes.dlc = 10;
  messages[PEDAL_CAN_RX] = mes;
  mes.dlc = 12;
  messages[PEDAL_CAN_TX] = mes;
  mes.dlc = 13;
  messages[PEDAL_CAN_FAULT] = mes;
  //////////////

  can_init(can_storage, can_settings);

  // doesn't need a receive handler
  // temp purely for testing purposes
  can_register_rx_default_handler(prv_rx_callback, NULL);
  //////////////

  return STATUS_CODE_OK;
}

bool pedal_can_process_event(Event *e) {
  if (e->id < NUM_PEDAL_CAN_EVENTS) {
    ///////JUST FOR TESTING
    can_transmit(&messages[e->id], NULL);
    LOG_DEBUG("Transmitted can message\n");
    //////

    return true;
  }
  return false;
}