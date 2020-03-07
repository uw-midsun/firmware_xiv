#pragma once

// This is the top level power fsm for the car. Transitions will raise events that will trigger
// power transition sequences: power_main_sequence, and power_aux_sequence.

#include <stdint.h>
#include "fsm.h"

typedef struct PowerFsmStorage {
  uint16_t fault_bitset;
  Fsm power_fsm;
} PowerFsmStorage;

StatusCode power_fsm_init(PowerFsmStorage *power_fsm);

bool power_fsm_process_event(PowerFsmStorage *power_fsm, const Event *event);
