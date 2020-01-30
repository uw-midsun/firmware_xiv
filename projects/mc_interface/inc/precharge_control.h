#pragma once
#include "status.h"
#include "event_queue.h"

typedef enum {
    PRECHARGE_STATE_INCOMPLETE = 0,
    PRECHARGE_STATE_PRECHARGING,
    PRECHARGE_STATE_COMPLETE,
    NUM_PRECHARGE_STATES
} PrechargeStates;

StatusCode precharge_init(void *context);

void precharge_fsm_process_event(const Event *e);
