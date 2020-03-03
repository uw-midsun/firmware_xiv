#pragma once

// This module is a giant, but simple state machine that ensures all the necessary
// steps for powering the car off of the main battery are executed successfully.
// The name "main" means that we are going to have the main battery be the source
// of power for the car. The SYSTEM_CAN_MESSAGE_POWER_ON_MAIN_SEQUENCE message is
// used for most of the steps of the power on sequence, other than the step involving
// the battery relays, which uses the SYSTEM_CAN_MESSAGE_SET_RELAY_STATES message.

// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "centre_console_fault_reason.h"
#include "exported_enums.h"
#include "fsm.h"
#include "precharge_monitor.h"
#include "status.h"

typedef struct PowerMainSequenceFsmStorage {
  Fsm sequence_fsm;
  EEPowerMainSequence current_sequence;
  PrechargeMonitor precharge_monitor_storage;
} PowerMainSequenceFsmStorage;

StatusCode power_main_sequence_init(PowerMainSequenceFsmStorage *power_fsm);

bool power_main_sequence_fsm_process_event(PowerMainSequenceFsmStorage *sequence_fsm,
                                           const Event *event);

uint32_t *test_get_ack_devices_lookup(void);
