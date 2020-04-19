#pragma once

// All the enums and definitions for fault reasons in centre console.

#include "stdint.h"

typedef enum {
  DRIVE_FSM_STATE_MACHINE = 0,
  POWER_MAIN_SEQUENCE_STATE_MACHINE,
  POWER_OFF_SEQUENCE_STATE_MACHINE,
  POWER_AUX_SEQUENCE_STATE_MACHINE,
  NUM_STATE_MACHINES,
} StateMachine;

// Used as the event data for the CENTRE_CONSOLE_POWER_EVENT_FAULT, and
// DRIVE_FSM_INPUT_EVENT_FAULT events.
typedef union StateTransitionFault {
  struct {
    StateMachine state_machine : 3;
    uint8_t fault_reason : 8;
  };
  uint16_t raw;
} StateTransitionFault;

typedef enum {
  DRIVE_FSM_TRANSITION_STEP_MCI_RELAY_STATE = 0,
  DRIVE_FSM_TRANSITION_STEP_PRECHARGE_TIMEOUT,
  DRIVE_FSM_TRANSITION_STEP_EBRAKE_STATE,
  DRIVE_FSM_TRANSITION_STEP_MCI_OUTPUT,
  NUM_DRIVE_FSM_FAULT_REASONS,
} DriveFsmTransitionStep;

typedef union {
  struct {
    DriveFsmTransitionStep step : 3;
    uint8_t state : 2;
  };
  uint8_t raw;
} DriveFsmFaultReason;
