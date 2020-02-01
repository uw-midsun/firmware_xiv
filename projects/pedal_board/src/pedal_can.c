#include <string.h>
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "log.h"
#include "pedal_events.h"

static StatusCode prv_rx_callback(const CanMessage *msg, void *context, CanAckStatus *ack_reply) {
  u_int8_t drive_state = 0;
  CAN_UNPACK_DRIVE_STATE(msg, &drive_state);
  event_raise(drive_state, 0);

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

static StatusCode prv_fault_rx_callback(const CanMessage *msg, void *context,
                                        CanAckStatus *ack_reply) {
  if (msg->msg_id != SYSTEM_CAN_MESSAGE_DRIVE_STATE) {
    event_raise(PEDAL_THROTTLE_EVENT_FAULT, 0);
  }

  // REMOVE THIS LATER
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
  can_init(can_storage, can_settings);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_DRIVE_STATE, prv_rx_callback, NULL);
  can_register_rx_default_handler(prv_fault_rx_callback, NULL);

  return STATUS_CODE_OK;
}

StatusCode pedal_can_process_event(Event *e) {
  if ((e->id == PEDAL_THROTTLE_EVENT_DATA) || (e->id == PEDAL_THROTTLE_EVENT_FAULT) ||
      (e->id == PEDAL_BRAKE_MONITOR_EVENT_FAULT)) {
    return CAN_TRANSMIT_THROTTLE_OUTPUT(e->id);
  } else if ((e->id == PEDAL_BRAKE_FSM_EVENT_PRESSED) ||
             (e->id == PEDAL_BRAKE_FSM_EVENT_RELEASED)) {
    return CAN_TRANSMIT_BRAKE(e->id);
  }
  return STATUS_CODE_OK;
}
