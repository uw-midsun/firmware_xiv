#include "drive_fsm.h"

#include "pedal_events.h"
#include "throttle.h"

FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_idle);

FSM_STATE_TRANSITION(state_drive) {
  FSM_ADD_TRANSITION(PEDAL_EVENT_THROTTLE_DISABLE, state_idle);
}

FSM_STATE_TRANSITION(state_idle) {
  FSM_ADD_TRANSITION(PEDAL_EVENT_THROTTLE_ENABLE, state_drive);
}

static void prv_state_drive_output(Fsm *fsm, const Event *e, void *context) {
  ThrottleStorage *storage = context;
  throttle_enable(storage);
}

static void prv_state_idle_output(Fsm *fsm, const Event *e, void *context) {
  ThrottleStorage *storage = context;
  throttle_disable(storage);
}

StatusCode drive_fsm_init(Fsm *fsm, ThrottleStorage *storage) {
  fsm_state_init(state_drive, prv_state_drive_output);
  fsm_state_init(state_idle, prv_state_idle_output);

  fsm_init(fsm, "Drive FSM", &state_drive, storage);

  return STATUS_CODE_OK;
}
