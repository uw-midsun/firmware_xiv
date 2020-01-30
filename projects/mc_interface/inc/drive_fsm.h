#pragma once
#include "status.h"

typedef enum {
    DRIVE_FSM_STATE_NEUTRAL = 0,
    DRIVE_FSM_STATE_DRIVE,
    DRIVE_FSM_STATE_REVERSE,
    NUM_DRIVE_FSM_STATES
} DriveState;

static StatusCode drive_fsm_init(void* context);

void drive_fsm_process_event(const Event *e);