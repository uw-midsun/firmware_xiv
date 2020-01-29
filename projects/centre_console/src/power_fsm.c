#include "power_fsm.h"
#include "centre_console_events.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "status.h"

#define POWER_FSM_NO_FAULT 0

FSM_DECLARE_STATE(power_state_main);
FSM_DECLARE_STATE(power_state_off);
FSM_DECLARE_STATE(power_state_aux);
FSM_DECLARE_STATE(power_state_turning_on_main);
FSM_DECLARE_STATE(power_state_turning_on_aux);
FSM_DECLARE_STATE(power_state_turning_off);
FSM_DECLARE_STATE(power_state_fault);

FSM_STATE_TRANSITION(power_state_off) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, power_state_turning_on_main);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_ON_AUX, power_state_turning_on_aux);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_main) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_OFF, power_state_turning_off);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_aux) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, power_state_turning_on_main);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_OFF, power_state_turning_off);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_turning_on_main) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_COMPLETE, power_state_main);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_turning_on_aux) {
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_COMPLETE, power_state_aux);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_turning_off) {
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_COMPLETE, power_state_off);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

static bool prv_guard_faults_cleared(const Fsm *fsm, const Event *e, void *context) {
  PowerFsmStorage *power_fsm = (PowerFsmStorage *)context;
  power_fsm->fault_bitset &= ~(e->data);
  bool faults_cleared = (power_fsm->fault_bitset == POWER_FSM_NO_FAULT);
  if (!faults_cleared) {
    event_raise(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, power_fsm->fault_bitset);
  }
  return faults_cleared;
}

FSM_STATE_TRANSITION(power_state_fault) {
  FSM_ADD_GUARDED_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, prv_guard_faults_cleared,
                             power_state_turning_on_aux);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

static void prv_power_state_turning_on_main(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_MAIN_SEQUENCE_EVENT_BEGIN, 0);
}

static void prv_power_state_turning_on_aux(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_AUX_SEQUENCE_EVENT_BEGIN, 0);
}

static void prv_state_turning_off_output(Fsm *fsm, const Event *e, void *context) {
  event_raise(POWER_OFF_SEQUENCE_EVENT_BEGIN, 0);
}

static void prv_state_fault_output(Fsm *fsm, const Event *e, void *context) {
  PowerFsmStorage *power_fsm = (PowerFsmStorage *)context;
  power_fsm->fault_bitset |= e->data;
  event_raise(CENTRE_CONSOLE_POWER_EVENT_PUBLISH_FAULT, power_fsm->fault_bitset);
}

bool power_fsm_process_event(PowerFsmStorage *power_fsm, const Event *event) {
  return fsm_process_event(&power_fsm->power_fsm, event);
}

StatusCode power_fsm_init(PowerFsmStorage *power_fsm) {
  fsm_state_init(power_state_turning_on_main, prv_power_state_turning_on_main);
  fsm_state_init(power_state_turning_on_aux, prv_power_state_turning_on_aux);
  fsm_state_init(power_state_turning_off, prv_state_turning_off_output);
  fsm_state_init(power_state_fault, prv_state_fault_output);
  fsm_init(&power_fsm->power_fsm, "power_fsm", &power_state_off, power_fsm);
  return STATUS_CODE_OK;
}
