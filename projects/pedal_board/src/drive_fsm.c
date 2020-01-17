#include "drive_fsm.h"

#include "pedal_events.h"
#include "throttle.h"

FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_neutral);

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(PEDAL_EVENT_THROTTLE_DISABLE, state_neutral);
  FSM_ADD_TRANSITION(PEDAL_THROTTLE_EVENT_FAULT, state_neutral);
}

FSM_STATE_TRANSITION(state_neutral) {
  FSM_ADD_TRANSITION(PEDAL_EVENT_THROTTLE_ENABLE, state_drive);
  FSM_ADD_TRANSITION(PEDAL_THROTTLE_EVENT_FAULT, state_neutral);
}

static void prv_state_drive_output(Fsm *fsm, const Event *e, void *context) {
  ThrottleStorage *storage = context;
  throttle_enable(storage);
}

static void prv_state_neutral_output(Fsm *fsm, const Event *e, void *context) {
  ThrottleStorage *storage = context;
  throttle_disable(storage);
}

StatusCode drive_fsm_init(Fsm *fsm, ThrottleStorage *storage) {
  fsm_state_init(state_drive, prv_state_drive_output);
  fsm_state_init(state_neutral, prv_state_neutral_output);

  fsm_init(fsm, "Drive_FSM", &state_drive, storage);

  return STATUS_CODE_OK;
}

bool drive_fsm_process_event(Fsm *drive_fsm, Event *e) {
  bool transitioned = fsm_process_event(drive_fsm, e); 

  return transitioned; 
}
