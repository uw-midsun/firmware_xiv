#pragma once

// FSM for managing the relay state.
// Requires the event queue, GPIO, and the DRV120 relay driver to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "event_queue.h"
#include "fsm.h"
#include "status.h"

#define MAX_RELAY_OPEN_EVENTS 8

typedef struct SolarFsmSettings {
  // Fault events which cause the relay to open.
  EventId relay_open_events[MAX_RELAY_OPEN_EVENTS];
  uint8_t num_relay_open_events;  // length of preceding array
} SolarFsmSettings;

typedef struct SolarFsmStorage {
  Fsm fsm;
  EventId relay_open_events[MAX_RELAY_OPEN_EVENTS];
  uint8_t num_relay_open_events;
} SolarFsmStorage;

// Initialize the FSM. The relay is initialized to closed.
StatusCode solar_fsm_init(SolarFsmStorage *storage, const SolarFsmSettings *settings);

// Process an event and return whether the relay changed state.
bool solar_fsm_process_event(SolarFsmStorage *storage, const Event *event);
