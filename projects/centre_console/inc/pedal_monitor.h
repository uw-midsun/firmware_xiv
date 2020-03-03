#pragma once

// This module uses pedal_rx to find the state of the pedal. Pedal state gets
// updated every PEDAL_STATE_UPDATE_FREQUENCY_MS milliseconds. Pedal state gets
// calculated by the PEDAL_STATE_THRESHOLD value.

// Requires CAN, GPIO, soft timers, event queue, and interrupts to be initialized.

#include "pedal_rx.h"

#define PEDAL_RX_TIMEOUT_MS 300
#define PEDAL_STATE_UPDATE_FREQUENCY_MS 100
#define PEDAL_STATE_THRESHOLD 50

typedef enum { PEDAL_STATE_PRESSED = 0, PEDAL_STATE_RELEASED, NUM_PEDAL_STATES } PedalState;

typedef struct PedalMonitorStorage {
  PedalRxStorage rx_storage;
  SoftTimerId timer_id;
  PedalState state;
} PedalMonitorStorage;

StatusCode pedal_monitor_init(void);

PedalState get_pedal_state(void);
