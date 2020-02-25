#pragma once

// This module is a simple state machine that ensures all the necessary steps for
// powering the car off of the aux battery are executed successfully.
// The name "aux" means that we are going to have the aux battery be the source
// of power for the car. The SYSTEM_CAN_MESSAGE_POWER_ON_AUX_SEQUENCE message is
// used for all of the steps of this sequence.

// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "exported_enums.h"
#include "fsm.h"
#include "status.h"

typedef struct PowerAuxSequence {
  EEPowerAuxSequence current_sequence;
  Fsm sequence_fsm;
} PowerAuxSequenceFsmStorage;

StatusCode power_aux_sequence_init(PowerAuxSequenceFsmStorage *storage);

bool power_aux_sequence_process_event(PowerAuxSequenceFsmStorage *storage, const Event *event);
