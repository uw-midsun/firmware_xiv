#include "power_fsm.h"
#include "can_transmit.h"
#include "centre_console_events.h"
#include "centre_console_fault_reason.h"
#include "critical_section.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "status.h"

FSM_DECLARE_STATE(power_state_main);
FSM_DECLARE_STATE(power_state_off);
FSM_DECLARE_STATE(power_state_aux);
FSM_DECLARE_STATE(power_state_transitioning);
FSM_DECLARE_STATE(power_state_fault);

FSM_STATE_TRANSITION(power_state_off) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, power_state_transitioning);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_ON_AUX, power_state_transitioning);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_main) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_OFF, power_state_transitioning);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_aux) {
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_ON_MAIN, power_state_transitioning);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_OFF, power_state_transitioning);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

FSM_STATE_TRANSITION(power_state_transitioning) {
  FSM_ADD_TRANSITION(POWER_MAIN_SEQUENCE_EVENT_COMPLETE, power_state_main);
  FSM_ADD_TRANSITION(POWER_AUX_SEQUENCE_EVENT_COMPLETE, power_state_aux);
  FSM_ADD_TRANSITION(POWER_OFF_SEQUENCE_EVENT_COMPLETE, power_state_off);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_FAULT, power_state_fault);
}

static bool prv_guard_clear_fault(const Fsm *fsm, const Event *e, void *context) {
  PowerFsmStorage *power_fsm = (PowerFsmStorage *)context;
  return power_fsm->previous_state == e->data;
}

FSM_STATE_TRANSITION(power_state_fault) {
  FSM_ADD_GUARDED_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, prv_guard_clear_fault,
                             power_state_off);
  FSM_ADD_GUARDED_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, prv_guard_clear_fault,
                             power_state_aux);
  FSM_ADD_GUARDED_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, prv_guard_clear_fault,
                             power_state_main);
  FSM_ADD_TRANSITION(CENTRE_CONSOLE_POWER_EVENT_OFF, power_state_transitioning);
}

// This function is called when we catch a bps fault
// we discharge main and switch to aux
// we the lockup firmware with an infinite loop
// until the fault is resolved and the car's been power cycled
static void prv_bps_fault(void) {
  CAN_TRANSMIT_DISCHARGE_PRECHARGE();
  critical_section_start();
  while (true) {
  }
}

static const char *s_power_state_names[] = {
  [POWER_STATE_TRANSITIONING] = "transitioning",
  [POWER_STATE_OFF] = "off",
  [POWER_STATE_MAIN] = "main",
  [POWER_STATE_AUX] = "aux",
  [POWER_STATE_FAULT] = "fault",
};

static void prv_state_fault_output(Fsm *fsm, const Event *e, void *context) {
  // Go back to previous state
  PowerFsmStorage *power_fsm = (PowerFsmStorage *)context;
  power_fsm->destination_state = power_fsm->previous_state;
  FaultReason fault = { .raw = e->data };
  power_fsm->current_state = POWER_STATE_FAULT;
  if (fault.fields.area != EE_CONSOLE_FAULT_AREA_BPS_HEARTBEAT) {
    CAN_TRANSMIT_STATE_TRANSITION_FAULT(fault.fields.area, fault.fields.reason);
    event_raise(CENTRE_CONSOLE_POWER_EVENT_CLEAR_FAULT, power_fsm->previous_state);
  } else {
    prv_bps_fault();
  }
  LOG_DEBUG("faulting, going to state %s\n", s_power_state_names[power_fsm->previous_state]);
}

static void prv_destination_state_output(Fsm *fsm, const Event *e, void *context) {
  PowerFsmStorage *power_fsm = (PowerFsmStorage *)context;
  power_fsm->current_state = power_fsm->destination_state;
  LOG_DEBUG("moved to state %s\n", s_power_state_names[power_fsm->current_state]);
}

static PowerState s_destination_lookup[] = {
  [CENTRE_CONSOLE_POWER_EVENT_OFF] = POWER_STATE_OFF,
  [CENTRE_CONSOLE_POWER_EVENT_ON_MAIN] = POWER_STATE_MAIN,
  [CENTRE_CONSOLE_POWER_EVENT_ON_AUX] = POWER_STATE_AUX
};

bool power_fsm_process_event(PowerFsmStorage *power_fsm, const Event *event) {
  if (CENTRE_CONSOLE_POWER_EVENT_OFF <= event->id &&
      event->id <= CENTRE_CONSOLE_POWER_EVENT_ON_AUX) {
    power_fsm->destination_state = s_destination_lookup[event->id];
  }
  return fsm_process_event(&power_fsm->power_fsm, event);
}

static EventId s_event_lookup[] = {
  [CENTRE_CONSOLE_POWER_EVENT_ON_MAIN] = POWER_MAIN_SEQUENCE_EVENT_BEGIN,
  [CENTRE_CONSOLE_POWER_EVENT_ON_AUX] = POWER_AUX_SEQUENCE_EVENT_BEGIN,
  [CENTRE_CONSOLE_POWER_EVENT_OFF] = POWER_OFF_SEQUENCE_EVENT_BEGIN,
};

static void prv_power_state_transitioning(Fsm *fsm, const Event *e, void *context) {
  PowerFsmStorage *power_fsm = (PowerFsmStorage *)context;
  power_fsm->previous_state = power_fsm->current_state;
  power_fsm->current_state = POWER_STATE_TRANSITIONING;
  event_raise_no_data(s_event_lookup[e->id]);
}

StatusCode power_fsm_init(PowerFsmStorage *power_fsm) {
  fsm_state_init(power_state_transitioning, prv_power_state_transitioning);
  fsm_state_init(power_state_fault, prv_state_fault_output);
  fsm_state_init(power_state_aux, prv_destination_state_output);
  fsm_state_init(power_state_main, prv_destination_state_output);
  fsm_state_init(power_state_off, prv_destination_state_output);
  power_fsm->current_state = POWER_STATE_OFF;
  fsm_init(&power_fsm->power_fsm, "power_fsm", &power_state_off, power_fsm);
  return STATUS_CODE_OK;
}

PowerState power_fsm_get_current_state(PowerFsmStorage *power_fsm) {
  return power_fsm->current_state;
}
