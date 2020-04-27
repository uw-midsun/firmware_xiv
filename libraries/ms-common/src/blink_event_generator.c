#include "blink_event_generator.h"

#define BLINK_EVENT_PRIORITY EVENT_PRIORITY_NORMAL

static BlinkerState prv_opposite_state(BlinkerState state) {
  return state == BLINKER_STATE_ON ? BLINKER_STATE_OFF : BLINKER_STATE_ON;
}

static uint16_t prv_state_to_value(BlinkerState state) {
  return state == BLINKER_STATE_ON ? 1 : 0;
}

StatusCode blink_event_generator_init(BlinkEventGeneratorStorage *storage,
                                      const BlinkEventGeneratorSettings *settings) {
  if (settings->default_state >= NUM_BLINKER_STATES) {
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->interval_us = settings->interval_us;
  storage->default_state = storage->current_state = settings->default_state;
  storage->callback = settings->callback;
  storage->callback_context = settings->callback_context;
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
  return STATUS_CODE_OK;
}

static bool prv_is_active(BlinkEventGeneratorStorage *storage) {
  return storage->timer_id != SOFT_TIMER_INVALID_TIMER;
}

static void prv_raise_blink_event_callback(SoftTimerId timer_id, void *context) {
  BlinkEventGeneratorStorage *storage = context;
  BlinkerState new_state = prv_opposite_state(storage->current_state);
  event_raise_priority(BLINK_EVENT_PRIORITY, storage->event_id, prv_state_to_value(new_state));
  storage->current_state = new_state;

  soft_timer_start(storage->interval_us, &prv_raise_blink_event_callback, storage,
                   &storage->timer_id);

  if (storage->callback) {
    storage->callback(new_state, storage->callback_context);
  }
}

StatusCode blink_event_generator_start(BlinkEventGeneratorStorage *storage, EventId event_id) {
  if (prv_is_active(storage)) {
    if (event_id == storage->event_id) {
      // restarting with the same event: do nothing to avoid a strange delay
      return STATUS_CODE_OK;
    }

    // don't have two blink generators going at the same time
    blink_event_generator_stop(storage);
  }
  storage->event_id = event_id;

  // raise an event immediately and start the soft timer
  prv_raise_blink_event_callback(SOFT_TIMER_INVALID_TIMER, storage);
  return STATUS_CODE_OK;
}

bool blink_event_generator_stop(BlinkEventGeneratorStorage *storage) {
  if (storage->current_state != storage->default_state) {
    // raise a final event to go back to the default state
    event_raise_priority(BLINK_EVENT_PRIORITY, storage->event_id,
                         prv_state_to_value(storage->default_state));
  }

  return blink_event_generator_stop_silently(storage);
}

bool blink_event_generator_stop_silently(BlinkEventGeneratorStorage *storage) {
  bool result = soft_timer_cancel(storage->timer_id);
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->current_state = storage->default_state;
  return result;
}
