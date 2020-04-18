#pragma once

// This is the top level power fsm for the car. Transitions will raise events that will trigger
// power transition sequences: power_main_sequence, power_off_sequence, and power_aux_sequence.

#include <stdint.h>
#include "fsm.h"

typedef enum {
  POWER_STATE_TRANSITIONING = 0,
  POWER_STATE_OFF,
  POWER_STATE_MAIN,
  POWER_STATE_AUX,
  POWER_STATE_FAULT,
  NUM_POWER_STATES
} PowerState;

typedef struct PowerFsmStorage {
  uint16_t fault_bitset;
  Fsm power_fsm;
  PowerState current_state;
  PowerState destination_state;
} PowerFsmStorage;

StatusCode power_fsm_init(PowerFsmStorage *power_fsm);

bool power_fsm_process_event(PowerFsmStorage *power_fsm, const Event *event);

PowerState power_fsm_get_current_state(PowerFsmStorage *power_fsm);
