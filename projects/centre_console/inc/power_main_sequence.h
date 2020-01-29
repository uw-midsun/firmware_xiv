#pragma once

#include "centre_console_fault_reason.h"
#include "exported_enums.h"
#include "fsm.h"
#include "power_main_precharge_monitor.h"
#include "status.h"

typedef struct PowerMainSequenceFsmStorage {
  Fsm sequence_fsm;
  EEPowerMainSequence current_sequence;
  PowerMainPrechargeMonitor precharge_monitor_storage;
} PowerMainSequenceFsmStorage;

StatusCode power_main_sequence_init(PowerMainSequenceFsmStorage *power_fsm);

bool power_main_sequence_fsm_process_event(PowerMainSequenceFsmStorage *sequence_fsm,
                                           const Event *event);
