#include "lights_signal_fsm.h"

#include <stdbool.h>
#include <stdint.h>

#include "can_transmit.h"
#include "event_queue.h"
#include "status.h"

FSM_DECLARE_STATE(state_none);                 // no lights active
FSM_DECLARE_STATE(state_left_signal);          // left signal active
FSM_DECLARE_STATE(state_right_signal);         // right signal active
FSM_DECLARE_STATE(state_hazard_signal);        // hazard active
FSM_DECLARE_STATE(state_hazard_left_signal);   // hazard active and left signal active
FSM_DECLARE_STATE(state_hazard_right_signal);  // hazard active and right signal active

static bool prv_guard_on(const Fsm *fsm, const Event *e, void *context) {
  return e->data == 1;
}

static bool prv_guard_off(const Fsm *fsm, const Event *e, void *context) {
  return e->data == 0;
}

// A bit of an abuse of the guard system: used to stop the blinker during sync event handling.
// This avoids adding ~5 extra states that just stop the blinker and go back.
static bool prv_stop_blinker(const Fsm *fsm, const Event *e, void *context) {
  SignalFsmStorage *storage = fsm->context;
  blink_event_generator_stop_silently(&storage->blink_event_generator);
  return true;
}

FSM_STATE_TRANSITION(state_none) {
  SignalFsmStorage *storage = fsm->context;
  FSM_ADD_GUARDED_TRANSITION(storage->signal_left_input_event, prv_guard_on, state_left_signal);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_right_input_event, prv_guard_on, state_right_signal);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_hazard_input_event, prv_guard_on, state_hazard_signal);
}

FSM_STATE_TRANSITION(state_left_signal) {
  SignalFsmStorage *storage = fsm->context;
  FSM_ADD_GUARDED_TRANSITION(storage->signal_left_input_event, prv_guard_off, state_none);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_hazard_input_event, prv_guard_on,
                             state_hazard_left_signal);
  if (storage->sync_behaviour == LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS) {
    FSM_ADD_GUARDED_TRANSITION(storage->sync_event, prv_stop_blinker, state_left_signal);
  }
}

FSM_STATE_TRANSITION(state_right_signal) {
  SignalFsmStorage *storage = fsm->context;
  FSM_ADD_GUARDED_TRANSITION(storage->signal_right_input_event, prv_guard_off, state_none);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_hazard_input_event, prv_guard_on,
                             state_hazard_right_signal);
  if (storage->sync_behaviour == LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS) {
    FSM_ADD_GUARDED_TRANSITION(storage->sync_event, prv_stop_blinker, state_right_signal);
  }
}

FSM_STATE_TRANSITION(state_hazard_signal) {
  SignalFsmStorage *storage = fsm->context;
  FSM_ADD_GUARDED_TRANSITION(storage->signal_left_input_event, prv_guard_on,
                             state_hazard_left_signal);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_right_input_event, prv_guard_on,
                             state_hazard_right_signal);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_hazard_input_event, prv_guard_off, state_none);
  if (storage->sync_behaviour == LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS) {
    FSM_ADD_GUARDED_TRANSITION(storage->sync_event, prv_stop_blinker, state_hazard_signal);
  }
}

FSM_STATE_TRANSITION(state_hazard_left_signal) {
  SignalFsmStorage *storage = fsm->context;
  FSM_ADD_GUARDED_TRANSITION(storage->signal_left_input_event, prv_guard_off, state_hazard_signal);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_hazard_input_event, prv_guard_off, state_left_signal);
  if (storage->sync_behaviour == LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS) {
    FSM_ADD_GUARDED_TRANSITION(storage->sync_event, prv_stop_blinker, state_hazard_left_signal);
  }
}

FSM_STATE_TRANSITION(state_hazard_right_signal) {
  SignalFsmStorage *storage = fsm->context;
  FSM_ADD_GUARDED_TRANSITION(storage->signal_right_input_event, prv_guard_off, state_hazard_signal);
  FSM_ADD_GUARDED_TRANSITION(storage->signal_hazard_input_event, prv_guard_off, state_right_signal);
  if (storage->sync_behaviour == LIGHTS_SYNC_BEHAVIOUR_RECEIVE_SYNC_MSGS) {
    FSM_ADD_GUARDED_TRANSITION(storage->sync_event, prv_stop_blinker, state_hazard_right_signal);
  }
}

static void prv_state_none_output(Fsm *fsm, const Event *e, void *context) {
  SignalFsmStorage *storage = context;
  blink_event_generator_stop(&storage->blink_event_generator);
}

static void prv_state_left_signal_output(Fsm *fsm, const Event *e, void *context) {
  SignalFsmStorage *storage = context;
  blink_event_generator_start(&storage->blink_event_generator, storage->signal_left_output_event);
}

static void prv_state_right_signal_output(Fsm *fsm, const Event *e, void *context) {
  SignalFsmStorage *storage = context;
  blink_event_generator_start(&storage->blink_event_generator, storage->signal_right_output_event);
}

static void prv_state_hazard_signal_output(Fsm *fsm, const Event *e, void *context) {
  SignalFsmStorage *storage = context;
  blink_event_generator_start(&storage->blink_event_generator, storage->signal_hazard_output_event);
}

static void prv_blink_event_raised_callback(BlinkerState new_state, void *context) {
  SignalFsmStorage *storage = context;

  // we only send sync events on rising edges
  if (storage->sync_behaviour == LIGHTS_SYNC_BEHAVIOUR_SEND_SYNC_MSGS &&
      new_state == BLINKER_STATE_ON) {
    storage->blink_counter++;
    if (storage->blink_counter >= storage->num_blinks_between_syncs) {
      // transmit sync event
      storage->blink_counter = 0;
      CAN_TRANSMIT_LIGHTS_SYNC();
    }
  }
}

StatusCode lights_signal_fsm_init(SignalFsmStorage *storage, const SignalFsmSettings *settings) {
  if (settings->sync_behaviour >= NUM_SYNC_BEHAVIOURS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  storage->signal_left_input_event = settings->signal_left_input_event;
  storage->signal_right_input_event = settings->signal_right_input_event;
  storage->signal_hazard_input_event = settings->signal_hazard_input_event;
  storage->signal_left_output_event = settings->signal_left_output_event;
  storage->signal_right_output_event = settings->signal_right_output_event;
  storage->signal_hazard_output_event = settings->signal_hazard_output_event;
  storage->sync_event = settings->sync_event;
  storage->sync_behaviour = settings->sync_behaviour;
  storage->num_blinks_between_syncs = settings->num_blinks_between_syncs;
  storage->blink_counter = 0;

  BlinkEventGeneratorSettings blinker_settings = {
    .interval_us = settings->blink_interval_us,
    .event_priority = settings->event_priority,
    .default_state = BLINKER_STATE_OFF,  // all lights default to off
    .callback = prv_blink_event_raised_callback,
    .callback_context = storage,
  };
  status_ok_or_return(
      blink_event_generator_init(&storage->blink_event_generator, &blinker_settings));

  fsm_init(&storage->fsm, "Lights Signal FSM", &state_none, storage);
  fsm_state_init(state_none, prv_state_none_output);
  fsm_state_init(state_left_signal, prv_state_left_signal_output);
  fsm_state_init(state_right_signal, prv_state_right_signal_output);
  fsm_state_init(state_hazard_signal, prv_state_hazard_signal_output);
  fsm_state_init(state_hazard_left_signal, prv_state_hazard_signal_output);
  fsm_state_init(state_hazard_right_signal, prv_state_hazard_signal_output);

  return STATUS_CODE_OK;
}

StatusCode lights_signal_fsm_process_event(SignalFsmStorage *storage, const Event *event) {
  fsm_process_event(&storage->fsm, event);
  return STATUS_CODE_OK;
}
