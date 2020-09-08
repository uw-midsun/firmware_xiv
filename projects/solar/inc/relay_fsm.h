#pragma once

// FSM for managing the relay state.
// Requires the event queue and the DRV120 relay driver to be initialized.

#include <stdbool.h>

#include "event_queue.h"
#include "fsm.h"
#include "status.h"

typedef struct RelayFsmStorage {
  Fsm fsm;
} RelayFsmStorage;

// Initialize the FSM. The relay is initialized to closed.
StatusCode relay_fsm_init(RelayFsmStorage *storage);

// Process an event and return whether the relay changed state.
bool relay_fsm_process_event(RelayFsmStorage *storage, const Event *event);

StatusCode relay_fsm_close(void);

StatusCode relay_fsm_open(void);
