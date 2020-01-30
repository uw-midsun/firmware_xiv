#pragma once

#include "exported_enums.h"
#include "fsm.h"
#include "status.h"

typedef struct PowerAuxSequence {
  EEPowerAuxSequence current_sequence;
  Fsm sequence_fsm;
} PowerAuxSequenceFsmStorage;

StatusCode power_aux_sequence_init(PowerAuxSequenceFsmStorage *storage);

bool power_aux_sequence_process_event(PowerAuxSequenceFsmStorage *storage, const Event *event);
