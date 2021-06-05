#pragma once

// FSM for managing the relay state.
// Requires the event queue and the DRV120 relay driver to be initialized.

#include <stdbool.h>

#include "drv120_relay.h"
#include "event_queue.h"
#include "fsm.h"
#include "soft_timer.h"
#include "solar_boards.h"
#include "status.h"

#define CURRENT_ASSERT_THRESHOLD_uA 10

#define RELAY_ASSERTION_DELAY_MS 10
#define DATA_STORE_ASSERTION_DELAY_MS 1000

typedef struct RelayFsmStorage {
  Fsm fsm;
  bool isErrCalled;
  uint32_t isSetCounter;
} RelayFsmStorage;

// Initialize the FSM. The relay is initialized to closed.
StatusCode relay_fsm_init(RelayFsmStorage *storage, SolarMpptCount mppt_count);

// Process an event and return whether the relay changed state.
bool relay_fsm_process_event(RelayFsmStorage *storage, const Event *event);

StatusCode relay_fsm_close(void);

StatusCode relay_fsm_open(void);

// Callback called on status pin interrupt
void relay_err_cb(const GpioAddress *address, void *context);
