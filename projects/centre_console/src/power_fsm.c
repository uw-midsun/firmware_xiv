#include "centre_console_events.h"
#include "fsm.h"
#include "status.h"
#include "power_fsm.h"
#include "event_queue.h"

#define POWER_FSM_NO_FAULT 0

FSM_DECLARE_STATE(power_state_on);
FSM_DECLARE_STATE(power_state_off);
FSM_DECLARE_STATE(power_state_turning_on);
FSM_DECLARE_STATE(power_state_turning_off);
FSM_DECLARE_STATE(power_state_fault);

FSM_STATE_TRANSITION(power_state_off) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_ON, power_state_turning_on);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_on) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_OFF, power_state_turning_off);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_turning_on) {
  FSM_ADD_TRANSITION(POWER_ON_SEQUENCE_EVENT_COMPLETE, power_state_on);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_turning_off) {
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_COMPLETE, power_state_off);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_fault) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, power_state_fault);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULTS_CLEARED, power_state_off);
}

static void prv_state_turning_on_output(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_ON_SEQUENCE_EVENT_BEGIN, 0);
}

static void prv_state_turning_off_output(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_OFF_SEQUENCE_EVENT_BEGIN, 0);
}

static void prv_state_fault_output(Fsm *fsm, const Event *e, void *context) {
  PowerFsmStorage *power_fsm = (PowerFsmStorage *) context;
  if (e->id == CENTRE_CONSOLE_POWER_EVENT_FAULT) {
    power_fsm->fault_bitset |= e->data;
  } else if (e->id == CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT){
    power_fsm->fault_bitset &= ~(e->data);
  }
  if (power_fsm->fault_bitset == POWER_FSM_NO_FAULT) {
    event_raise(CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, 0);
  } else {
    event_raise(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, power_fsm->fault_bitset);
  }
}

bool power_fsm_process_event(PowerFsmStorage *power_fsm, const Event *event) {
  return fsm_process_event(&power_fsm->power_fsm, event);
}

StatusCode power_fsm_init(PowerFsmStorage *power_fsm) {
  fsm_state_init(power_state_turning_on, prv_state_turning_on_output);
  fsm_state_init(power_state_turning_off, prv_state_turning_off_output);
  fsm_state_init(power_state_fault, prv_state_fault_output);
  fsm_init(&power_fsm->power_fsm, "power_fsm", &power_state_off, power_fsm);
  return STATUS_CODE_OK;
}
