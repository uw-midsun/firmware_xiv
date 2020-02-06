#include "blink_event_generator.h"

StatusCode blink_event_generator_init(BlinkEventGeneratorStorage *storage,
                                      const BlinkEventGeneratorSettings *settings) {
  if (settings->first_value > 1) {
    // the only valid blink values are 0 and 1
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->interval_us = settings->interval_us;
  storage->next_value = storage->first_value = settings->first_value;
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
  return STATUS_CODE_OK;
}

static bool prv_is_active(BlinkEventGeneratorStorage *storage) {
  return storage->timer_id != SOFT_TIMER_INVALID_TIMER;
}

static void prv_raise_blink_event(SoftTimerId timer_id, void *context) {
  BlinkEventGeneratorStorage *storage = context;
  event_raise_priority(EVENT_PRIORITY_NORMAL, storage->event_id, storage->next_value);
  storage->next_value = !storage->next_value;

  soft_timer_start(storage->interval_us, &prv_raise_blink_event, storage, &storage->timer_id);
}

StatusCode blink_event_generator_start(BlinkEventGeneratorStorage *storage, EventId event_id) {
  if (prv_is_active(storage)) {
    // don't have two blink generators going at the same time
    blink_event_generator_stop(storage);
  }
  storage->event_id = event_id;
  return soft_timer_start(storage->interval_us, &prv_raise_blink_event, storage,
                          &storage->timer_id);
}

bool blink_event_generator_stop(BlinkEventGeneratorStorage *storage) {
  bool result = soft_timer_cancel(storage->timer_id);
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
  storage->next_value = storage->first_value;
  return result;
}
