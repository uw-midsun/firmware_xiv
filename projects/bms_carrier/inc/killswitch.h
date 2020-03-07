#pragma once
// Handles the killswitch
// Requires GPIO, GPIO interrupts, and interrupts to be initialized.
//
// Raises a fault event if the killswitch is hit as an input.
// Allows bypassing the killswitch as an output.
#include "bps_heartbeat.h"
#include "debouncer.h"
#include "gpio.h"

typedef struct KillswitchStorage {
  DebouncerStorage debouncer;
} KillswitchStorage;

// Set the killswitch up to fault if hit. Assumes the killswitch is active-low.
StatusCode killswitch_init(KillswitchStorage *storage, const GpioAddress *killswitch,
                           BpsHeartbeatStorage *bps_heartbeat);

// Bypass the killswitch. It does not need to be initialized.
StatusCode killswitch_bypass(const GpioAddress *killswitch);
