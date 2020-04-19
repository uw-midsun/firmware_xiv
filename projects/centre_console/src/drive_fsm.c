#include "drive_fsm.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "ebrake_tx.h"
#include "fsm.h"
#include "log.h"
#include "precharge_monitor.h"
#include "relay_tx.h"

static DriveState s_drive_state = NUM_DRIVE_STATES;

FSM_DECLARE_STATE(state_fault);
FSM_DECLARE_STATE(state_set_precharge);
FSM_DECLARE_STATE(state_neutral_precharged);
FSM_DECLARE_STATE(state_neutral_discharged);
FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_reverse);
FSM_DECLARE_STATE(state_parking);
FSM_DECLARE_STATE(state_set_motorcontroller_output);
FSM_DECLARE_STATE(state_set_ebrake);

FSM_STATE_TRANSITION(state_neutral_discharged) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PARKING, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_REVERSE, state_set_precharge);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DRIVE, state_set_precharge);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_NEUTRAL, state_set_precharge);
}

FSM_STATE_TRANSITION(state_neutral_precharged) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PARKING, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_REVERSE, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DRIVE, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_REVERSE, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PARKING, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_NEUTRAL, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_parking) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_REVERSE, state_set_precharge);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_NEUTRAL, state_set_precharge);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DRIVE, state_set_precharge);
}

FSM_STATE_TRANSITION(state_set_precharge) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PRECHARGE_COMPLETED, state_set_ebrake);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DISCHARGE_COMPLETED, state_parking);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_reverse) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_PARKING, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_NEUTRAL, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_DRIVE, state_set_motorcontroller_output);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_fault) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_EBRAKE_PRESSED, state_parking);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_RELEASED, state_neutral_discharged);
}

FSM_STATE_TRANSITION(state_set_motorcontroller_output) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_DRIVE, state_drive);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_REVERSE, state_reverse);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_OFF,
                     state_neutral_precharged);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

FSM_STATE_TRANSITION(state_set_ebrake) {
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED, state_set_precharge);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED, state_neutral_precharged);
  FSM_ADD_TRANSITION(DRIVE_FSM_INPUT_EVENT_FAULT, state_fault);
}

typedef struct DestinationTransitionInfo {
  EventId mci_output_success_event;
  EEEbrakeState ebrake_state;
  EventId fsm_output_event_id;
  EEDriveOutput mci_drive_output;
} DestinationTransitionInfo;

static DestinationTransitionInfo s_destination_transition_lookup[NUM_DRIVE_STATES] = {
  [DRIVE_STATE_NEUTRAL] = { .mci_output_success_event =
                                DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_OFF,
                            .ebrake_state = EE_EBRAKE_STATE_RELEASED,
                            .mci_drive_output = EE_DRIVE_OUTPUT_OFF,
                            .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_NEUTRAL },
  [DRIVE_STATE_PARKING] = { .mci_output_success_event =
                                DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_OFF,
                            .ebrake_state = EE_EBRAKE_STATE_PRESSED,
                            .mci_drive_output = EE_DRIVE_OUTPUT_OFF,
                            .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_PARKING },
  [DRIVE_STATE_DRIVE] = { .mci_output_success_event =
                              DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_DRIVE,
                          .ebrake_state = EE_EBRAKE_STATE_RELEASED,
                          .mci_drive_output = EE_DRIVE_OUTPUT_DRIVE,
                          .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_DRIVE },
  [DRIVE_STATE_REVERSE] = { .mci_output_success_event =
                                DRIVE_FSM_INPUT_EVENT_MCI_SET_OUTPUT_DESTINATION_REVERSE,
                            .ebrake_state = EE_EBRAKE_STATE_RELEASED,
                            .mci_drive_output = EE_DRIVE_OUTPUT_REVERSE,
                            .fsm_output_event_id = DRIVE_FSM_OUTPUT_EVENT_REVERSE },
};

static void prv_fault_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  CAN_TRANSMIT_DISCHARGE_PRECHARGE();
  EventId id = (storage->ebrake_storage.current_state == EE_EBRAKE_STATE_PRESSED)
                   ? DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_EBRAKE_PRESSED
                   : DRIVE_FSM_INPUT_EVENT_FAULT_RECOVER_RELEASED;
  storage->destination = NUM_DRIVE_STATES;
  event_raise(id, 0);
}

static void prv_set_motorcontroller_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  storage->current_state = DRIVE_STATE_TRANSITIONING;
  DestinationTransitionInfo info = s_destination_transition_lookup[storage->destination];
  StateTransitionFault fault = {
    .state_machine = DRIVE_FSM_STATE_MACHINE,
    .fault_reason = (DriveFsmFaultReason){ .step = DRIVE_FSM_TRANSITION_STEP_MCI_OUTPUT,
                                           .state = info.mci_drive_output }
                        .raw
  };
  RetryTxRequest tx_req = { .completion_event_id = info.mci_output_success_event,
                            .completion_event_data = 0,
                            .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
                            .fault_event_data = fault.raw,
                            .retry_indefinitely = false };
  mci_output_tx_drive_output(&storage->mci_output_storage, &tx_req, info.mci_drive_output);
}

static void prv_set_ebrake_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  storage->current_state = DRIVE_STATE_TRANSITIONING;
  DestinationTransitionInfo info = s_destination_transition_lookup[storage->destination];

  StateTransitionFault fault = {
    .state_machine = DRIVE_FSM_STATE_MACHINE,
    .fault_reason = (DriveFsmFaultReason){ .step = DRIVE_FSM_TRANSITION_STEP_EBRAKE_STATE,
                                           .state = info.ebrake_state }
                        .raw
  };

  RetryTxRequest tx_req = { .completion_event_id = (info.ebrake_state == EE_EBRAKE_STATE_PRESSED)
                                                       ? DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_PRESSED
                                                       : DRIVE_FSM_INPUT_EVENT_MCI_EBRAKE_RELEASED,
                            .completion_event_data = 0,
                            .fault_event_id = DRIVE_FSM_INPUT_EVENT_FAULT,
                            .fault_event_data = fault.raw,
                            .retry_indefinitely = false };
  ebrake_tx_brake_state(&storage->ebrake_storage, &tx_req, info.ebrake_state);
}

static void prv_drive_fsm_destination_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  if (storage->destination >= NUM_DRIVE_STATES) {
    return;
  }
  storage->current_state = storage->destination;
  DestinationTransitionInfo info = s_destination_transition_lookup[storage->destination];
  event_raise(info.fsm_output_event_id, 0);
}

static DriveFsmInputEvent s_neutral_destination_event_lookup[NUM_DRIVE_STATES] = {
  [DRIVE_STATE_PARKING] = DRIVE_FSM_INPUT_EVENT_PARKING,
  [DRIVE_STATE_DRIVE] = DRIVE_FSM_INPUT_EVENT_DRIVE,
  [DRIVE_STATE_REVERSE] = DRIVE_FSM_INPUT_EVENT_REVERSE
};

static void prv_drive_fsm_set_precharge_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  storage->current_state = DRIVE_STATE_TRANSITIONING;
  if (storage->destination == DRIVE_STATE_PARKING) {
    CAN_TRANSMIT_DISCHARGE_PRECHARGE();
    event_raise(DRIVE_FSM_INPUT_EVENT_DISCHARGE_COMPLETED, 0);
    return;
  }
  precharge_monitor_start(&storage->precharge_monitor_storage);
}

static void prv_drive_fsm_neutral_precharged_output(Fsm *fsm, const Event *e, void *context) {
  DriveFsmStorage *storage = (DriveFsmStorage *)context;
  if (storage->destination != DRIVE_STATE_NEUTRAL) {
    event_raise(s_neutral_destination_event_lookup[storage->destination], 0);
    return;
  }
  prv_drive_fsm_destination_output(fsm, e, context);
}

StatusCode drive_fsm_init(DriveFsmStorage *storage) {
  fsm_state_init(state_drive, prv_drive_fsm_destination_output);
  fsm_state_init(state_reverse, prv_drive_fsm_destination_output);
  fsm_state_init(state_parking, prv_drive_fsm_destination_output);
  fsm_state_init(state_set_precharge, prv_drive_fsm_set_precharge_output);
  fsm_state_init(state_neutral_precharged, prv_drive_fsm_neutral_precharged_output);
  fsm_state_init(state_set_motorcontroller_output, prv_set_motorcontroller_output);
  fsm_state_init(state_set_ebrake, prv_set_ebrake_output);
  fsm_state_init(state_fault, prv_fault_output);
  status_ok_or_return(ebrake_tx_init(&storage->ebrake_storage));
  status_ok_or_return(mci_output_init(&storage->mci_output_storage));
  storage->current_state = DRIVE_STATE_NEUTRAL;
  Event precharge_success_event = { .id = DRIVE_FSM_INPUT_EVENT_PRECHARGE_COMPLETED, .data = 0 };

  StateTransitionFault fault = { .state_machine = DRIVE_FSM_STATE_MACHINE,
                                 .fault_reason =
                                     (DriveFsmFaultReason){
                                         .step = DRIVE_FSM_TRANSITION_STEP_PRECHARGE_TIMEOUT,
                                     }
                                         .raw };

  Event precharge_fault_event = { .id = DRIVE_FSM_INPUT_EVENT_FAULT, .data = fault.raw };
  status_ok_or_return(precharge_monitor_init(&storage->precharge_monitor_storage,
                                             PRECHARGE_TIMEOUT_S * 1000, &precharge_success_event,
                                             &precharge_fault_event));
  fsm_init(&(storage->drive_fsm), "Drive FSM", &state_neutral_discharged, storage);
  return STATUS_CODE_OK;
}

static DriveState s_destination_lookup[] = { [DRIVE_FSM_INPUT_EVENT_NEUTRAL] = DRIVE_STATE_NEUTRAL,
                                             [DRIVE_FSM_INPUT_EVENT_PARKING] = DRIVE_STATE_PARKING,
                                             [DRIVE_FSM_INPUT_EVENT_DRIVE] = DRIVE_STATE_DRIVE,
                                             [DRIVE_FSM_INPUT_EVENT_REVERSE] =
                                                 DRIVE_STATE_REVERSE };

bool drive_fsm_process_event(DriveFsmStorage *storage, Event *e) {
  if (e->id >= DRIVE_FSM_INPUT_EVENT_NEUTRAL && e->id <= DRIVE_FSM_INPUT_EVENT_DRIVE) {
    storage->destination = s_destination_lookup[e->id];
  }
  return fsm_process_event(&(storage->drive_fsm), e);
}

DriveState drive_fsm_get_global_state(DriveFsmStorage *storage) {
  return storage->current_state;
}
