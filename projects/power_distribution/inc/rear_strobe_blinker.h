#pragma once

// Blinks the strobe light, controlled by POWER_DISTRIBUTION_STROBE_EVENT.
// This should only be called in rear power distribution.
// Requires interrupts, soft timers, and the event queue to be initialized.

#include "event_queue.h"

typedef struct {
  uint32_t strobe_blink_delay_us;
} RearStrobeBlinkerSettings;

StatusCode rear_strobe_blinker_init(RearStrobeBlinkerSettings *settings);

StatusCode rear_strobe_blinker_process_event(Event *e);
