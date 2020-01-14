#pragma once

#include "fsm.h"
#include "status.h"
#include "centre_console_fault_reason.h"

typedef struct PowerOnSequenceFsmStorage {
  Fsm sequence_fsm;
  CentreConsoleFaultReason ack_fault_reason;
  EventId success_ack_next_event;
} PowerOnSequenceFsmStorage;

StatusCode power_on_sequence_fsm_init(PowerOnSequenceFsmStorage *power_fsm);

bool power_on_sequence_fsm_process_event(PowerOnSequenceFsmStorage *sequence_fsm, const Event *event);
