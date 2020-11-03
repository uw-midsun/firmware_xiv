#include "power_aux_sequence.h"
#include "can_ack.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "fsm.h"
#include "log.h"

FSM_DECLARE_STATE(state_none);
FSM_DECLARE_STATE(state_fault);
FSM_DECLARE_STATE(state_confirm_aux_status);
FSM_DECLARE_STATE(state_turn_on_everything);
FSM_DECLARE_STATE(state_power_aux_complete);

FSM_STATE_TRANSITION(state_none) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_BEGIN, state_confirm_aux_status);
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_confirm_aux_status) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_AUX_STATUS_OK, state_turn_on_everything);
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_turn_on_everything) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_TURNED_ON_EVERYTHING, state_power_aux_complete);
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_power_aux_complete) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_COMPLETE, state_none);
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_fault) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, state_none);
}

static EventId s_next_event_lookup[NUM_EE_POWER_AUX_SEQUENCES] = {
  [EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] = POWER_AUX_SEQUENCE_EVENT_AUX_STATUS_OK,
  [EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = POWER_AUX_SEQUENCE_EVENT_TURNED_ON_EVERYTHING,
};

static EventId s_ack_devices_lookup[NUM_EE_POWER_AUX_SEQUENCES] = { 0 };

// for some reason defining them as a static variable didn't work
void prv_init_ack_lookup(void) {
  // s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] =
  //     CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR);
  // s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = CAN_ACK_EXPECTED_DEVICES(
  //     SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_REAR, SYSTEM_CAN_DEVICE_POWER_DISTRIBUTION_FRONT);
  s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] =
    CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_BABYDRIVER);
  s_ack_devices_lookup[EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] =
    CAN_ACK_EXPECTED_DEVICES(SYSTEM_CAN_DEVICE_BABYDRIVER);
}

static uint8_t s_ack_count[NUM_EE_POWER_AUX_SEQUENCES] = { 0 };

static void prv_init_ack_count(void) {
  for (uint8_t i = 0; i < NUM_EE_POWER_AUX_SEQUENCES; i++) {
    s_ack_count[i] = 1;
  }
  // s_ack_count[EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = 2;
  s_ack_count[EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = 1;
}

static void prv_state_none(Fsm *fsm, const Event *e, void *context) {
  PowerAuxSequenceFsmStorage *storage = (PowerAuxSequenceFsmStorage *)context;
  prv_init_ack_count();
  storage->current_sequence = EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS;
}

static const char *s_aux_sequence_names[] = {
  [EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS] = "confirm aux status",
  [EE_POWER_AUX_SEQUENCE_TURN_ON_EVERYTHING] = "turn on everything",
};

static StatusCode prv_can_simple_ack(CanMessageId msg_id, uint16_t device, CanAckStatus status,
                                     uint16_t num_remaining, void *context) {
  PowerAuxSequenceFsmStorage *storage = (PowerAuxSequenceFsmStorage *)context;
  if (status == CAN_ACK_STATUS_TIMEOUT) {
    LOG_DEBUG("ack timed out\n");
  }
  EventId next_event = s_next_event_lookup[storage->current_sequence];
  s_ack_count[storage->current_sequence]--;
  if (s_ack_count[storage->current_sequence]) {
    LOG_DEBUG("ack from %d for sequence %s\n", device,
              s_aux_sequence_names[storage->current_sequence]);
    return STATUS_CODE_OK;
  }
  if (num_remaining == 0 && status == CAN_ACK_STATUS_OK) {
    storage->current_sequence++;
    LOG_DEBUG("got enough acks, continuing sequence\n");
    return event_raise(next_event, 0);
  }
  LOG_DEBUG("bad ack dev %d\n", device);
  return event_raise(POWER_AUX_SEQUENCE_EVENT_FAULT, storage->current_sequence);
}

static void prv_state_power_aux_sequence_output(Fsm *fsm, const Event *e, void *context) {
  PowerAuxSequenceFsmStorage *storage = (PowerAuxSequenceFsmStorage *)context;
  CanAckRequest ack_req = {
    .callback = prv_can_simple_ack,
    .context = storage,
    .expected_bitset = s_ack_devices_lookup[storage->current_sequence],
  };
  LOG_DEBUG("current sequence: %s\n", s_aux_sequence_names[storage->current_sequence]);
  CAN_TRANSMIT_POWER_ON_AUX_SEQUENCE(&ack_req, storage->current_sequence);
}

static void prv_state_power_aux_complete(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("aux sequence complete\n");
  event_raise(POWER_AUX_SEQUENCE_EVENT_COMPLETE, 0);
}

static void prv_state_fault(Fsm *fsm, const Event *e, void *context) {
  FaultReason reason = { .fields = { .area = EE_CONSOLE_FAULT_AREA_POWER_AUX, .reason = e->data } };
  event_raise(CENTRE_CONSOLE_POWER_EVENT_FAULT, reason.raw);
}

StatusCode power_aux_sequence_init(PowerAuxSequenceFsmStorage *storage) {
  storage->current_sequence = EE_POWER_AUX_SEQUENCE_CONFIRM_AUX_STATUS;
  prv_init_ack_count();
  prv_init_ack_lookup();
  fsm_state_init(state_none, prv_state_none);
  fsm_state_init(state_confirm_aux_status, prv_state_power_aux_sequence_output);
  fsm_state_init(state_turn_on_everything, prv_state_power_aux_sequence_output);
  fsm_state_init(state_power_aux_complete, prv_state_power_aux_complete);
  fsm_state_init(state_fault, prv_state_fault);
  fsm_init(&storage->sequence_fsm, "power_aux_sequence_fsm", &state_none, storage);
  return STATUS_CODE_OK;
}

bool power_aux_sequence_process_event(PowerAuxSequenceFsmStorage *storage, const Event *e) {
  return fsm_process_event(&storage->sequence_fsm, e);
}
