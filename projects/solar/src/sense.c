#include "sense.h"
#include <stddef.h>
#include "data_store.h"
#include "soft_timer.h"

static SenseCallback s_callbacks[MAX_SENSE_CALLBACKS];
static void *s_contexts[MAX_SENSE_CALLBACKS];
static uint8_t s_num_callbacks = 0;

static uint32_t s_period_us;

static SoftTimerId s_timer_id;

static void prv_do_sense_cycle(SoftTimerId timer_id, void *context) {
  // call each callback in order
  for (uint8_t i = 0; i < s_num_callbacks; i++) {
    SenseCallback callback = s_callbacks[i];
    if (callback) {
      callback(s_contexts[i]);
    }
  }
  data_store_done();

  soft_timer_start(s_period_us, prv_do_sense_cycle, NULL, &s_timer_id);
}

StatusCode sense_init(SenseSettings *settings) {
  if (settings == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_period_us = settings->sense_period_us;
  s_num_callbacks = 0;  // reset callback stack upon reinitialization
  return STATUS_CODE_OK;
}

StatusCode sense_register(SenseCallback callback, void *callback_context) {
  if (s_num_callbacks >= MAX_SENSE_CALLBACKS) {
    return STATUS_CODE_RESOURCE_EXHAUSTED;
  }
  if (callback == NULL) {
    return STATUS_CODE_INVALID_ARGS;
  }
  s_callbacks[s_num_callbacks] = callback;
  s_contexts[s_num_callbacks] = callback_context;
  s_num_callbacks++;
  return STATUS_CODE_OK;
}

void sense_start(void) {
  // the timer id and context are never used in |prv_do_sense_cycle|
  prv_do_sense_cycle(NULL, NULL);
}

bool sense_stop(void) {
  return soft_timer_cancel(s_timer_id);
}
