#include "blink_event_generator.h"

StatusCode blink_event_generator_init(BlinkEventGeneratorStorage *storage,
                                      const BlinkEventGeneratorSettings *settings) {
  if (settings->first_value > 1) {
    // the only valid blink values are 0 and 1
    return STATUS_CODE_INVALID_ARGS;
  }

  storage->interval_us = settings->interval_us;
  storage->event_id = settings->event_id;
  storage->next_value = settings->first_value;
  return STATUS_CODE_OK;
}

static void prv_raise_blink_event(SoftTimerId timer_id, void *context) {
  BlinkEventGeneratorStorage *storage = context;
  event_raise_priority(EVENT_PRIORITY_NORMAL, storage->event_id, storage->next_value);
  storage->next_value = !storage->next_value;

  soft_timer_start(storage->interval_us, &prv_raise_blink_event, storage, &storage->timer_id);
}

StatusCode blink_event_generator_start(BlinkEventGeneratorStorage *storage) {
  return soft_timer_start(storage->interval_us, &prv_raise_blink_event, storage,
                          &storage->timer_id);
}

bool blink_event_generator_stop(BlinkEventGeneratorStorage *storage) {
  return soft_timer_cancel(storage->timer_id);
}
