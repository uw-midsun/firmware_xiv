#pragma once

// FSM for handling transitions between signal states.
// Uses blink_event_generator to raise user-defined signal left, right, and hazard output events.
// Input events are also user-defined.
// Requires interrupts, soft timers, and the event queue to be initialized.

#include "blink_event_generator.h"
#include "event_queue.h"
#include "fsm.h"
#include "status.h"

typedef struct {
  // Each event must have a data field of 1 for on or 0 for off.
  EventId signal_left_input_event;
  EventId signal_right_input_event;
  EventId signal_hazard_input_event;
  EventId signal_left_output_event;
  EventId signal_right_output_event;
  EventId signal_hazard_output_event;
  uint32_t blink_interval_us;
} SignalFsmSettings;

typedef struct {
  EventId signal_left_input_event;
  EventId signal_right_input_event;
  EventId signal_hazard_input_event;
  EventId signal_left_output_event;
  EventId signal_right_output_event;
  EventId signal_hazard_output_event;
  Fsm fsm;
  BlinkEventGeneratorStorage blink_event_generator;
} SignalFsmStorage;

StatusCode lights_signal_fsm_init(SignalFsmStorage *storage, const SignalFsmSettings *settings);

StatusCode lights_signal_fsm_process_event(SignalFsmStorage *storage, const Event *event);
