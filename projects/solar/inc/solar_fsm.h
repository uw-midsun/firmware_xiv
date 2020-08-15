#pragma once

// FSM for managing the relay state. Handles SYSTEM_CAN_MESSAGE_SET_RELAY_STATES.
// Requires the event queue, GPIO, interrupts, soft timers, CAN, and the DRV120 relay driver to be
// initialized.

#include <stdbool.h>
#include <stdint.h>

#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "status.h"

#define MAX_RELAY_OPEN_FAULTS 8

typedef struct SolarFsmSettings {
  // Faults which cause the relay to open.
  EESolarFault relay_open_faults[MAX_RELAY_OPEN_FAULTS];
  uint8_t num_relay_open_faults;  // length of preceding array
} SolarFsmSettings;

typedef struct SolarFsmStorage {
  Fsm fsm;
  EESolarFault relay_open_faults[MAX_RELAY_OPEN_FAULTS];
  uint8_t num_relay_open_faults;
} SolarFsmStorage;

// Initialize the FSM. The relay is initialized to closed.
StatusCode solar_fsm_init(SolarFsmStorage *storage, const SolarFsmSettings *settings);

// Process an event and return whether the relay changed state.
bool solar_fsm_process_event(SolarFsmStorage *storage, const Event *event);
