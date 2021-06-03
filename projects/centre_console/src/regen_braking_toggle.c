#include "regen_braking_toggle.h"

#include "can.h"
#include "can_ack.h"
#include "can_msg_defs.h"
#include "can_pack.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "status.h"

static bool regen_braking_state;

static StatusCode prv_ack_callback(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                   uint16_t num_remaining, void *context) {
  // if ACK fails, revert state
  if (status != CAN_ACK_STATUS_OK) {
    regen_braking_state = !regen_braking_state;
    CAN_TRANSMIT_REGEN_BRAKING(NULL, regen_braking_state);
  }
  return STATUS_CODE_OK;
}

static StatusCode prv_process_request(bool state) {
  CanAckRequest regen_ack_request = { .callback = prv_ack_callback,
                                      .context = NULL,
                                      .expected_bitset = CAN_ACK_EXPECTED_DEVICES(
                                          SYSTEM_CAN_DEVICE_MOTOR_CONTROLLER) };
  return CAN_TRANSMIT_REGEN_BRAKING(&regen_ack_request, state);
}

static StatusCode prv_toggle_callback(const CanMessage *msg, void *context,
                                      CanAckStatus *ack_reply) {
  regen_braking_state = !regen_braking_state;
  return prv_process_request(regen_braking_state);
}

StatusCode regen_braking_init() {
  regen_braking_state = false;
  return can_register_rx_handler(SYSTEM_CAN_MESSAGE_REGEN_BRAKING_TOGGLE_REQUEST,
                                 prv_toggle_callback, NULL);
}

bool get_regen_braking_state() {
  return regen_braking_state;
}

StatusCode set_regen_braking(bool state) {
  if (state != regen_braking_state) {
    regen_braking_state = state;
    return prv_process_request(regen_braking_state);
  }
  return STATUS_CODE_OK;
}

bool regen_braking_process_event(Event *e) {
  if (e != NULL && e->id == POWER_MAIN_SEQUENCE_EVENT_COMPLETE) {
    regen_braking_state = true;
    prv_process_request(true);
    return true;
  }
  return false;
}
