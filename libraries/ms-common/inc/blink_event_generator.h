#pragma once

// Takes an event ID with a data field and generates events toggling the data field between 1 and 0.
// Requires interrupts, soft timers, and the event queue to be initialized.

#include "event_queue.h"
#include "soft_timer.h"

typedef enum {
  BLINKER_STATE_ON = 0,
  BLINKER_STATE_OFF,
  NUM_BLINKER_STATES,
} BlinkerState;

typedef struct {
  uint32_t interval_us;
  BlinkerState default_state;  // if not specified, the default state is BLINKER_STATE_ON
} BlinkEventGeneratorSettings;

typedef struct {
  uint32_t interval_us;
  EventId event_id;
  BlinkerState default_state;
  BlinkerState current_state;
  SoftTimerId timer_id;
} BlinkEventGeneratorStorage;

// Initialize the generator instance with the given settings.
StatusCode blink_event_generator_init(BlinkEventGeneratorStorage *storage,
                                      const BlinkEventGeneratorSettings *settings);

// Start generating blink events with the corresponding event ID.
// The first event is raised |storage->interval_us| after this call.
// If the generator is currently blinking, it will stop before blinking with this event ID.
StatusCode blink_event_generator_start(BlinkEventGeneratorStorage *storage, EventId event_id);

// Stop generating blink events and return whether it was stopped.
bool blink_event_generator_stop(BlinkEventGeneratorStorage *storage);
