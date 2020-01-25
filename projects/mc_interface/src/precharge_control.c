#include "precharge_control.h"

#include "fsm.h"
#include "gpio.h"

FSM_DECLARE_STATE(precharge_incomplete);
FSM_DECLARE_STATE(precharge_precharging);
FSM_DECLARE_STATE(precharge_complete);

FSM_STATE_TRANSITION(precharge_incomplete) {
    FSM_ADD_TRANSITION(PRECHARGE_STATE_PRECHARGING, precharge_precharging);
}
FSM_STATE_TRANSITION(precharge_precharging) {
    FSM_ADD_TRANSITION(PRECHARGE_STATE_INCOMPLETE, precharge_incomplete);
    FSM_ADD_TRANSITION(PRECHARGE_STATE_COMPLETE, precharge_complete);
}
FSM_STATE_TRANSITION(precharge_complete) {
    FSM_ADD_TRANSITION(PRECHARGE_STATE_INCOMPLETE, precharge_incomplete);
}

static void prv_state_incomplete_output(Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    storage->precharge_state = PRECHARGE_STATE_INCOMPLETE;
}
static void prv_state_precharging_output(Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    storage->precharge_state = PRECHARGE_STATE_PRECHARGING;
}
static void prv_state_complete_output(Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    storage->precharge_state = PRECHARGE_STATE_COMPLETE;
}

static Fsm s_precharge_fsm;
static void prv_init_precharge_fsm(void *context) {
    fsm_state_init(precharge_incomplete, prv_fsm_precharge_incomplete);
    fsm_state_init(precharge_precharging, prv_fsm_precharge_precharging);
    fsm_state_init(precharge_complete, prv_fsm_precharge_complete);
    fsm_init(&s_precharge_fsm, "precharge_fsm", &precharge_incomplete, context);
}

void precharge_fsm_process_event(const Event *e) {
    fsm_process_event(&s_precharge_fsm, e);
}

static StatusCode precharge_init(void *context) {
    prv_init_precharge_fsm(context);
    //TODO: init the gpio pin for sending precharge command and interrupt
        //then either raise events for switching state
    return STATUS_CODE_OK;
}

