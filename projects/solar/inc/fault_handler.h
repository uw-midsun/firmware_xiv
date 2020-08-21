#pragma once

// Handles and TXs solar faults.
// Requires GPIO, interrupts, soft timers, the event queue, CAN, and relay_fsm to be initialized.

#include <stdint.h>

#include "exported_enums.h"
#include "status.h"

#define MAX_RELAY_OPEN_FAULTS 8

typedef struct FaultHandlerSettings {
  // Faults which cause the relay to open.
  EESolarFault relay_open_faults[MAX_RELAY_OPEN_FAULTS];
  size_t num_relay_open_faults;  // length of preceding array
} FaultHandlerSettings;

StatusCode fault_handler_init(FaultHandlerSettings *settings);

StatusCode fault_handler_raise_fault(EESolarFault fault, uint8_t fault_data);
