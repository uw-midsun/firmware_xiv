#pragma once

// Generic sense module.
// Requires interrupts, soft timers, and the event queue to be initialized.

// This module operates a "sense cycle": periodically, all registered sense callbacks will be run,
// then |data_store_done| will be called to notify data consumers that new data is available.

#include <stdbool.h>
#include <stdint.h>
#include "status.h"

#define MAX_SENSE_CALLBACKS 32

// Callback implementations should read data and call |data_store_set| with each data point.
// Implementations should avoid blocking - if data isn't ready, log it, possibly raise a fault
// event, and don't call |data_store_set|.
typedef void (*SenseCallback)(void *context);

typedef struct {
  // Waiting period between sense cycles in microseconds.
  uint32_t sense_period_us;
} SenseSettings;

// Initialize the module with the given settings.
StatusCode sense_init(SenseSettings *settings);

// Register a callback to be run on each sense cycle.
// This should only be called by modules implementing the callback (e.g. sense_voltage).
StatusCode sense_register(SenseCallback callback, void *callback_context);

// Start the sense cycle. The first round of sense callbacks will begin immediately.
void sense_start(void);

// Stop the sense cycle, return whether it was stopped.
bool sense_stop(void);
