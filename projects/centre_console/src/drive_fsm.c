#include "drive_fsm.h"
#include "centre_console_events.h"
#include "fsm.h"
#include "log.h"
#include "relay_tx.h"
#include "ebrake_tx.h"
#include "centre_console_fault_reason.h"

static DriveState s_drive_state = NUM_DRIVE_STATES;

FSM_DECLARE_STATE(state_fault);
FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_reverse);
FSM_DECLARE_STATE(state_parking);
FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_set_relays);
FSM_DECLARE_STATE(state_set_ebrake);

FSM_STATE_TRANSITION(state_neutral) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PARKING, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_REVERSE, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DRIVE, state_set_relays);
}

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PARKING, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_REVERSE, state_reverse);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_NEUTRAL, state_set_relays);
}

FSM_STATE_TRANSITION(state_parking) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_REVERSE, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_NEUTRAL, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DRIVE, state_set_ebrake);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PARKING, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_NEUTRAL, state_set_relays);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DRIVE, state_drive);
}

FSM_STATE_TRANSITION(state_fault) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL, state_neutral);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

static bool prv_destination_guard(const Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  return e->data == storage->destination;
}

FSM_STATE_TRANSITION(state_set_relays) {
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_DRIVE, prv_destination_guard, state_drive);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE, prv_destination_guard, state_reverse);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL, prv_destination_guard, state_neutral);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_PARKING, prv_destination_guard, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_set_ebrake) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED, state_parking);
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED_DESTINATION_DRIVE_REVERSE, prv_destination_guard, state_set_relays); // destination: drive, reverse
  FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED_DESTINATION_NEUTRAL, prv_destination_guard, state_neutral);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}


typedef struct DestinationTransitionInfo {
  EventId relay_event_id;
  EventId ebrake_event_id;
  uint16_t data;
  EEChargerSetRelayState relay_state;
  EEEbrakeState ebrake_state;
  EventId fsm_output_event_id;
} DestinationTransitionInfo;

static DestinationTransitionInfo s_destination_transition_lookup[NUM_DRIVE_STATES] = {
  [DRIVE_STATE_NEUTRAL] = {
    .relay_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL,
    .ebrake_event_id = DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED_DESTINATION_NEUTRAL,
    .data = DRIVE_STATE_NEUTRAL,
    .relay_state = EE_RELAY_STATE_OPEN,
    .ebrake_state = EE_EBRAKE_STATE_RELEASED,
    .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_NEUTRAL
  },
  [DRIVE_STATE_PARKING] = {
    .relay_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_PARKING,
    .ebrake_event_id = DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED,
    .data = DRIVE_STATE_PARKING,
    .relay_state = EE_RELAY_STATE_OPEN,
    .ebrake_state = EE_EBRAKE_STATE_PRESSED,
    .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_PARKING
  },
  [DRIVE_STATE_DRIVE] = {
    .relay_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_DRIVE,
    .ebrake_event_id = DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED_DESTINATION_DRIVE_REVERSE,
    .data = DRIVE_STATE_DRIVE,
    .relay_state = EE_RELAY_STATE_CLOSE,
    .ebrake_state = EE_EBRAKE_STATE_RELEASED,
    .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_DRIVE
  },
  [DRIVE_STATE_REVERSE] = {
    .relay_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_CLOSED_DESTINATION_REVERSE,
    .ebrake_event_id = DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED_DESTINATION_DRIVE_REVERSE,
    .data = DRIVE_STATE_REVERSE,
    .relay_state = EE_RELAY_STATE_CLOSE,
    .ebrake_state = EE_EBRAKE_STATE_RELEASED,
    .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_REVERSE
  }
};

static void prv_fault_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  storage->destination = DRIVE_STATE_NEUTRAL;
  DriveFsmFault fault = {
    .fault_reason = DRIVE_FSM_FAULT_REASON_MCI_RELAY_STATE,
    .fault_state = EE_RELAY_STATE_OPEN
  };
  RelayTxRequest tx_req = {
    .state = EE_RELAY_STATE_OPEN,
    .completion_event_id = DRIVE_FSM_INPUT_EVENT_MCI_RELAYS_OPENED_DESTINATION_NEUTRAL,
    .completion_event_data = 0,
    .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
    .fault_event_data = fault.raw,
    .retry_indefinitely = true
  };
  relay_tx_relay_state_and_raise_event(&storage->relay_storage, &tx_req);
}

static void prv_set_relays_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  DestinationTransitionInfo info = s_destination_transition_lookup[storage->destination];
  DriveFsmFault fault = {
    .fault_reason = DRIVE_FSM_FAULT_REASON_MCI_RELAY_STATE,
    .fault_state = info.relay_state
  };
  RelayTxRequest tx_req = {
    .state = info.relay_state,
    .completion_event_id = info.relay_event_id,
    .completion_event_data = info.data,
    .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
    .fault_event_data = fault.raw,
    .retry_indefinitely = false
  };
  relay_tx_relay_state_and_raise_event(&storage->relay_storage, &tx_req);
}

static void prv_set_ebrake_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  DestinationTransitionInfo info = s_destination_transition_lookup[storage->destination];
  DriveFsmFault fault = {
    .fault_reason = DRIVE_FSM_FAULT_REASON_EBRAKE_STATE,
    .fault_state = info.ebrake_state
  };
  EbrakeTxRequest tx_req = {
    .state = info.ebrake_state,
    .completion_event_id = info.ebrake_event_id,
    .completion_event_data = info.data,
    .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
    .fault_event_data = fault.raw,
    .retry_indefinitely = false
  };
  ebrake_tx_state_and_raise_event(&storage->ebrake_storage, &tx_req);
}

static void prv_drive_fsm_destination_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  s_drive_state = storage->destination;
  DestinationTransitionInfo info = s_destination_transition_lookup[storage->destination];
  event_raise(info.fsm_output_event_id, 0);
}

StatusCode drive_fsm_init(DriveFsmStorage *storage) {
  fsm_state_init(state_drive, prv_drive_fsm_destination_output);
  fsm_state_init(state_reverse, prv_drive_fsm_destination_output);
  fsm_state_init(state_parking, prv_drive_fsm_destination_output);
  fsm_state_init(state_neutral, prv_drive_fsm_destination_output);
  fsm_state_init(state_set_relays, prv_set_relays_output);
  fsm_state_init(state_set_ebrake, prv_set_ebrake_output);
  fsm_state_init(state_fault, prv_fault_output);
  status_ok_or_return(relay_tx_init(&storage->relay_storage));
  status_ok_or_return(ebrake_tx_init(&storage->ebrake_storage));
  fsm_init(&(storage->drive_fsm), "Drive FSM", &state_neutral, storage);
  return STATUS_CODE_OK;
}

static DriveState s_destination_lookup[] = {
  [DRIVE_FSM_INPUT_EVENT_NEUTRAL] = DRIVE_STATE_NEUTRAL,
  [DRIVE_FSM_INPUT_EVENT_PARKING] = DRIVE_STATE_PARKING,
  [DRIVE_FSM_INPUT_EVENT_DRIVE] = DRIVE_STATE_DRIVE,
  [DRIVE_FSM_INPUT_EVENT_REVERSE] = DRIVE_STATE_REVERSE
};

bool drive_fsm_process_event(DriveFsmStorage *storage, Event *e) { 
  if (e->id >= DRIVE_FSM_INPUT_EVENT_NEUTRAL && e->id <= DRIVE_FSM_INPUT_EVENT_DRIVE) {
    storage->destination = s_destination_lookup[e->id];
  }
  return fsm_process_event(&(storage->drive_fsm), e);
}

DriveState* drive_fsm_get_global_state(DriveFsmStorage *storage) {
  return &s_drive_state;
}

