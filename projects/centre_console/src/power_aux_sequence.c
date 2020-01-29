#include "power_aux_sequence.h"
#include "can_ack.h"
#include "centre_console_events.h"
#include "fsm.h"

FSM_DECLARE_STATE(state_none);
FSM_DECLARE_STATE(state_confirm_aux_status);
FSM_DECLARE_STATE(state_turn_on_everything);
FSM_DECLARE_STATE(state_power_aux_complete);

FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_BEGIN, state_confirm_aux_status);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_none);
}

FSM_STATE_TRANSITION(state_confirm_aux_status) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_AUX_STATUS_OK, state_turn_on_everything);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_none);
}

FSM_STATE_TRANSITION(state_turn_on_everything) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_TURNED_ON_EVERYTHING, state_power_aux_complete);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_none);
}

FSM_STATE_TRANSITION(state_power_aux_complete) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_COMPLETE, state_none);
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_FAULT, state_none);
}

static EventId s_next_event_lookup[NUM_EE_POWER_AUX_SEQUENCES] = {
  [EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] = POWER_AUX_SEQUENCE_EVENT_AUX_STATUS_OK,
  [EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = POWER_AUX_SEQUENCE_EVENT_TURNED_ON_EVERYTHING,
};

static EventId s_ack_devices_lookup[NUM_EE_POWER_AUX_SEQUENCES] = { 0 };

// for some reason defining them as a static variable didn't work
void prv_init_ack_lookup(void) {
  s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] =
      CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = CAN_ACK_EXPECTED_DEVICES(
      SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);
}

static void prv_state_none(Fsm *fsm, const Event *e, void *context) {
  PowerAuxSequenceFsmStorage *storage = (PowerAuxSequenceFsmStorage *)context;
  storage->current_sequence = EE_POWER_MAIN_SEQUENCE_CONFIRM_AUX_STATUS;
}

static StatusCode prv_can_simple_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                     uint16_t num_remaining, void *context) {
  PowerAuxSequenceFsmStorage *storage = (PowerAuxSequenceFsmStorage *)context;
  EventId next_event = s_next_event_lookup[storage->current_sequence];
  if (num_remaining == 0 && status == CAN_ACK_STATUS_OK) {
    storage->current_sequence++;
    return event_raise(next_event, 0);
  }
  return event_raise(POWER_MAIN_SEQUENCE_EVENT_FAULT, storage->current_sequence);
}

static void prv_state_power_aux_sequence_output(Fsm *fsm, const Event *e, void *context) {
  PowerAuxSequenceFsmStorage *storage = (PowerAuxSequenceFsmStorage *)context;
  CanAckRequest ack_req = {
    .callback = prv_can_simple_ack,
    .context = storage,
    .expected_bitset = s_ack_devices_lookup[storage->current_sequence],
  };
  // CAN_TRANSMIT_POWER_AUX_SEQUENCE(current_sequence, &ack_req);
}

static void prv_state_power_aux_complete(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_AUX_SEQUENCE_EVENT_COMPLETE, 0);
}

StatusCode power_aux_sequence_init(PowerAuxSequenceFsmStorage *storage) {
  prv_init_ack_lookup();
  fsm_state_init(state_none, prv_state_none);
  fsm_state_init(state_confirm_aux_status, prv_state_power_aux_sequence_output);
  fsm_state_init(state_turn_on_everything, prv_state_power_aux_sequence_output);
  fsm_state_init(state_power_aux_complete, prv_state_power_aux_complete);
  fsm_init(&storage->sequence_fsm, "power_aux_sequence_fsm", &state_none, storage);
  return STATUS_CODE_OK;
}

bool power_aux_sequence_process_event(PowerAuxSequenceFsmStorage *storage, const Event *e) {
  return fsm_process_event(&storage->sequence_fsm, e);
}
