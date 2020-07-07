#pragma once

// Blinks the strobe light, controlled by POWER_DISTRIBUTION_STROBE_EVENT.
// This should only be called in rear power distribution.
// Requires interrupts, soft timers, and the event queue to be initialized.

// Is it ok to hardcode the event?

#include "event_queue.h"

typedef struct {
  uint32_t strobe_blink_delay_us;
} RearPowerDistributionStrobeBlinkerSettings;

StatusCode rear_power_distribution_strobe_blinker_init(
    RearPowerDistributionStrobeBlinkerSettings *settings);

StatusCode rear_power_distribution_strobe_blinker_process_event(Event *e);
