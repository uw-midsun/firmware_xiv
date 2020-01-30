#include <string.h>
#include "can.h"
#include "can_transmit.h"
#include "event_queue.h"
#include "log.h"
#include "pedal_events.h"

// just for testing
CanMessage messages[NUM_PEDAL_CAN_EVENTS];
// just for testing
static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  // again purely for testing
  event_raise(msg->msg_id, msg->data);

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
    .source_id = can_storage->device_id,
    .type = CAN_MSG_TYPE_DATA,
    .msg_id = PEDAL_CAN_EVENT_BRAKE_PRESSED,
    .dlc = 0,
    .data = 0,
  };
  messages[PEDAL_CAN_EVENT_BRAKE_PRESSED] = mes;
  mes.msg_id = PEDAL_CAN_EVENT_BRAKE_RELEASED;
  messages[PEDAL_CAN_EVENT_BRAKE_RELEASED] = mes;
  mes.msg_id = PEDAL_EVENT_THROTTLE_READING;
  messages[PEDAL_EVENT_THROTTLE_READING] = mes;
  mes.msg_id = PEDAL_THROTTLE_EVENT_DATA;
  messages[PEDAL_THROTTLE_EVENT_DATA] = mes;
  mes.msg_id = PEDAL_THROTTLE_EVENT_FAULT;
  messages[PEDAL_THROTTLE_EVENT_FAULT] = mes;
  mes.msg_id = PEDAL_EVENT_THROTTLE_ENABLE;
  messages[PEDAL_EVENT_THROTTLE_ENABLE] = mes;
  mes.msg_id = PEDAL_EVENT_THROTTLE_DISABLE;
  messages[PEDAL_EVENT_THROTTLE_DISABLE] = mes;
  mes.msg_id = PEDAL_BRAKE_MONITOR_EVENT_FAULT;
  messages[PEDAL_BRAKE_MONITOR_EVENT_FAULT] = mes;
  mes.msg_id = PEDAL_CAN_RX;
  messages[PEDAL_CAN_RX] = mes;
  mes.msg_id = PEDAL_CAN_TX;
  messages[PEDAL_CAN_TX] = mes;
  mes.msg_id = PEDAL_CAN_FAULT;
  messages[PEDAL_CAN_FAULT] = mes;

  can_init(can_storage, can_settings);

  // doesn't need a receive handler
  // temp purely for testing purposes
  can_register_rx_default_handler(prv_rx_callback, NULL);

  return STATUS_CODE_OK;
}

StatusCode pedal_can_process_event(Event *e) {
  if (e->id == PEDAL_THROTTLE_EVENT_DATA) {
    return CAN_TRANSMIT_THROTTLE_OUTPUT(e->data);
  } else if (e->id == PEDAL_BRAKE_MONITOR_DATA) {
    return CAN_TRANSMIT_BRAKE(e->data);
  } else if ((e->id < NUM_PEDAL_BRAKE_FSM_EVENTS) && (e->id > NUM_PEDAL_DRIVE_INPUT_EVENTS)) {
    return CAN_TRANSMIT_DRIVE_STATE(e->id - PEDAL_BRAKE_FSM_EVENT_PRESSED);
  }
  //TESTING
  if (e->id < NUM_PEDAL_CAN_EVENTS) {
    messages[e->id].data = e->data;
    can_transmit(&messages[e->id], NULL);
    LOG_DEBUG("Transmitted can message\n");
    return true;
  }
  return false;
}
