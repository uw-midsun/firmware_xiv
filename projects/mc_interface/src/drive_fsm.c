#include "fsm.h"
#include "status.h"
#include "event_queue.h"
#include "motor_controller.h"
#include "precharge_control.h"

#include "drive_fsm.h"

FSM_DECLARE_STATE(state_neutral);
FSM_DECLARE_STATE(state_drive);
FSM_DECLARE_STATE(state_reverse);

static bool prv_guard_reverse(const Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    if (storage->precharge_state != PRECHARGE_STATE_COMPLETE) {
        return false;
    }
    return storage->vehicle_velocity[MOTOR_CONTROLLER_LEFT] < 20.f; //TODO: find a value for this (RPM)
}

static bool prv_guard_drive(const Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    if (storage->precharge_state != PRECHARGE_STATE_COMPLETE) {
        return false;
    }
    //We don't want to go straight to drive from reverse if the motor
    //is spinning too fast
    return storage->vehicle_velocity[MOTOR_CONTROLLER_LEFT] > -20.f; //TODO: check if this is valid
}

FSM_STATE_TRANSITION(state_neutral) {
    FSM_ADD_TRANSITION(DRIVE_FSM_STATE_DRIVE, state_drive);
    FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_STATE_REVERSE, prv_guard_reverse, state_reverse);
}

FSM_STATE_TRANSITION(state_drive) {
    FSM_ADD_TRANSITION(DRIVE_FSM_STATE_NEUTRAL, state_neutral);
    FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_STATE_REVERSE, prv_guard_reverse, state_reverse);
}

FSM_STATE_TRANSITION(state_reverse) {
    FSM_ADD_TRANSITION(DRIVE_FSM_STATE_NEUTRAL, state_neutral);
    FSM_ADD_GUARDED_TRANSITION(DRIVE_FSM_STATE_DRIVE, prv_guard_drive, state_drive);
}

//We probable want drive state CAN broadcast here
static void prv_state_neutral_output(Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    storage->drive_state = DRIVE_FSM_STATE_NEUTRAL;
}
static void prv_state_drive_output(Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    storage->drive_state = DRIVE_FSM_STATE_DRIVE;
}
static void prv_state_reverse_output(Fsm *fsm, const Event *e, void *context) {
    MotorControllerStorage *storage = context;
    //TODO: figure out how to send an ack? or just send a normal CAN message
    storage->drive_state = DRIVE_FSM_STATE_REVERSE;
}

static Fsm s_drive_fsm;
static void prv_init_drive_fsm(void *context) {
    fsm_state_init(state_neutral, prv_state_neutral_output);
    fsm_state_init(state_drive, prv_state_drive_output);
    fsm_state_init(state_reverse, prv_state_reverse_output);
    fsm_init(&s_drive_fsm, "drive_fsm", &state_neutral, context);
}

void drive_fsm_process_event(const Event *e) {
    fsm_process_event(&s_drive_fsm, e);
}

StatusCode drive_fsm_init(void* context) {
    prv_init_drive_fsm(context);
    return STATUS_CODE_OK;
}