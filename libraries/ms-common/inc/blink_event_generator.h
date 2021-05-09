#pragma once

// Takes an event ID with a data field and generates events toggling the data field between 1 and 0.
// Requires interrupts, soft timers, and the event queue to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "event_queue.h"
#include "soft_timer.h"

typedef enum {
  BLINKER_STATE_OFF = 0,
  BLINKER_STATE_ON,
  NUM_BLINKER_STATES,
} BlinkerState;

// new_state is the state of the most recently raised event.
typedef void (*BlinkEventGeneratorCallback)(BlinkerState new_state, void *context);

typedef struct BlinkEventGeneratorSettings {
  uint32_t interval_us;
  EventPriority event_priority;          // must be set! (defaults to EVENT_PRIORITY_HIGHEST)
  BlinkerState default_state;            // defaults to BLINKER_STATE_OFF
  BlinkEventGeneratorCallback callback;  // can be null; else, called after each event raised
  void *callback_context;
} BlinkEventGeneratorSettings;

typedef struct BlinkEventGeneratorStorage {
  uint32_t interval_us;
  EventId event_id;
  EventPriority event_priority;
  BlinkEventGeneratorCallback callback;
  void *callback_context;
  BlinkerState default_state;
  BlinkerState current_state;
  SoftTimerId timer_id;
} BlinkEventGeneratorStorage;

// Initialize the generator instance with the given settings.
StatusCode blink_event_generator_init(BlinkEventGeneratorStorage *storage,
                                      const BlinkEventGeneratorSettings *settings);

// Start generating blink events with the corresponding event ID.
// The first event is raised immediately after this call; it will be away from the default state.
// If the generator is currently blinking, it will stop before blinking with this event ID.
// If we're currently blinking with the same event as |event_id|, do nothing so as to avoid an
// abnormal delay.
// If you want to make sure that the blink event generator restarts and raises an event immediately,
// call blink_event_generator_stop before calling this function.
StatusCode blink_event_generator_start(BlinkEventGeneratorStorage *storage, EventId event_id);

// Stop generating blink events and return whether it was stopped.
// If the timer is running and we aren't currently in the default state, raise a final event
// immediately to move back to the default state.
bool blink_event_generator_stop(BlinkEventGeneratorStorage *storage);

// Stop generating blink events and return whether it was stopped.
// Unlike blink_event_generator_stop, don't raise a final event to move back to the default state.
// This means we may be stuck outside the default state.
bool blink_event_generator_stop_silently(BlinkEventGeneratorStorage *storage);
