#pragma once

// Takes an event ID with a data field and generates events toggling the data field between 1 and 0.
// Requires interrupts, soft timers, and the event queue to be initialized.

#include "event_queue.h"
#include "soft_timer.h"

typedef struct {
  uint32_t interval_us;
  EventId event_id;
  uint16_t first_value;  // value of the first event; must be 1 or 0, default is 0 if not specified
} BlinkEventGeneratorSettings;

typedef struct {
  uint32_t interval_us;
  EventId event_id;
  uint16_t next_value;
  SoftTimerId timer_id;
} BlinkEventGeneratorStorage;

// Initialize the generator instance with the given settings.
StatusCode blink_event_generator_init(BlinkEventGeneratorStorage *storage,
                                      const BlinkEventGeneratorSettings *settings);

// Start generating blink events. The first event is raised |storage->interval_us| after this call.
StatusCode blink_event_generator_start(BlinkEventGeneratorStorage *storage);

// Stop generating blink events and return whether it was stopped.
bool blink_event_generator_stop(BlinkEventGeneratorStorage *storage);
