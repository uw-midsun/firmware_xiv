#pragma once

#include "can.h"
#include "event_queue.h"
#include "status.h"

typedef enum {
  DRIVE_FSM_STATE_NEUTRAL = 0,
  DRIVE_FSM_STATE_DRIVE,
  DRIVE_FSM_STATE_REVERSE,
  NUM_DRIVE_FSM_STATES
} DriveFsmState;

StatusCode drive_fsm_init(void *context);

StatusCode fault_rx(const CanMessage *msg, void *context, CanAckStatus *ack_status);

bool drive_fsm_process_event(const Event *e);
