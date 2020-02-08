#pragma once

// FSM for handling transitions between signal states.
// Uses blink_event_generator to raise FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_LEFT,
// FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_RIGHT, and
// FRONT_POWER_DISTRIBUTION_GPIO_EVENT_SIGNAL_HAZARD.
// Transition events are user-defined.
// Requires interrupts, soft timers, and the event queue to be initialized.

#include "blink_event_generator.h"
#include "event_queue.h"
#include "fsm.h"
#include "status.h"

typedef struct {
  // Each event must have a data field of 1 for on or 0 for off.
  EventId signal_left_event;
  EventId signal_right_event;
  EventId signal_hazard_event;
  uint32_t blink_interval_us;  // should this be input or #defined?
} SignalFsmSettings;

typedef struct {
  EventId signal_left_event;
  EventId signal_right_event;
  EventId signal_hazard_event;
  Fsm fsm;
  BlinkEventGeneratorStorage blink_event_generator;
} SignalFsmStorage;

// configure output events?
StatusCode lights_signal_fsm_init(SignalFsmStorage *storage, const SignalFsmSettings *settings);

StatusCode lights_signal_fsm_process_event(SignalFsmStorage *storage, const Event *event);
