#pragma once

// FSM for handling transitions between signal states.
// Uses blink_event_generator to raise user-defined signal left, right, and hazard output events.
// Input events are also user-defined.
// Requires interrupts, soft timers, and the event queue to be initialized.

#include "blink_event_generator.h"
#include "event_queue.h"
#include "fsm.h"
#include "status.h"

typedef enum {
  SYNC_BEHAVIOUR_NO_SYNC = 0,
  SYNC_BEHAVIOUR_SEND_SYNC_EVENTS,
  SYNC_BEHAVIOUR_RECEIVE_SYNC_EVENTS,
  NUM_SYNC_BEHAVIOURS,
} SignalFsmSyncBehaviour;

typedef struct {
  // Each event must have a data field of 1 for on or 0 for off.
  EventId signal_left_input_event;
  EventId signal_right_input_event;
  EventId signal_hazard_input_event;
  EventId signal_left_output_event;
  EventId signal_right_output_event;
  EventId signal_hazard_output_event;
  EventId sync_event;  // sync event to raise (if sending) or receive (if receiving)
  SignalFsmSyncBehaviour sync_behaviour;
  uint16_t num_blinks_between_syncs;  // only needs defined if using SYNC_BEHAVIOUR_SEND_SYNC_EVENTS
  uint32_t blink_interval_us;
} SignalFsmSettings;

typedef struct {
  EventId signal_left_input_event;
  EventId signal_right_input_event;
  EventId signal_hazard_input_event;
  EventId signal_left_output_event;
  EventId signal_right_output_event;
  EventId signal_hazard_output_event;
  EventId sync_event;
  SignalFsmSyncBehaviour sync_behaviour;
  uint16_t num_blinks_between_syncs;
  Fsm fsm;
  BlinkEventGeneratorStorage blink_event_generator;
  uint16_t blink_counter;
} SignalFsmStorage;

StatusCode lights_signal_fsm_init(SignalFsmStorage *storage, const SignalFsmSettings *settings);

StatusCode lights_signal_fsm_process_event(SignalFsmStorage *storage, const Event *event);
