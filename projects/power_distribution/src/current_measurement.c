#include "current_measurement.h"

#include <stdint.h>
#include <string.h>

#include "log.h"
#include "output.h"

static CurrentMeasurementConfig *s_config;
static CurrentMeasurementStorage s_storage = { 0 };
static SoftTimerId s_timer_id;

static uint32_t s_interval_us;
static CurrentMeasurementCallback s_callback;
static void *s_callback_context;

static void prv_measure_currents(SoftTimerId timer_id, void *context) {
  // read from each output
  for (uint8_t i = 0; i < s_config->num_outputs_to_read; i++) {
    Output output = s_config->outputs_to_read[i];
    if (output >= NUM_OUTPUTS) {
      // shouldn't be possible, we caught them in init
      continue;
    }
    StatusCode code = output_read_current(output, &s_storage.measurements[output]);
    if (!status_ok(code)) {
      LOG_WARN("Could not read current from output %d (index %d): code %d\n", output, i, code);
    }
  }

  if (s_callback != NULL) {
    s_callback(s_callback_context);
  }

  StatusCode code = soft_timer_start(s_interval_us, prv_measure_currents, NULL, &s_timer_id);
  if (!status_ok(code)) {
    LOG_CRITICAL("Could not restart current measurement cycle! Status code=%d\n", code);
  }
}

StatusCode current_measurement_init(CurrentMeasurementSettings *settings) {
  s_config = settings->config;
  s_interval_us = settings->interval_us;
  s_callback = settings->callback;
  s_callback_context = settings->callback_context;

  memset(&s_storage, 0, sizeof(s_storage));

  // catch any invalid outputs early for fast failure
  for (uint8_t i = 0; i < s_config->num_outputs_to_read; i++) {
    if (s_config->outputs_to_read[i] >= NUM_OUTPUTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
  }

  // measure the currents immediately; the callback doesn't use the timer id it's passed
  prv_measure_currents(SOFT_TIMER_INVALID_TIMER, NULL);

  return STATUS_CODE_OK;
}

CurrentMeasurementStorage *current_measurement_get_storage(void) {
  return &s_storage;
}

StatusCode current_measurement_stop(void) {
  soft_timer_cancel(s_timer_id);
  s_timer_id = SOFT_TIMER_INVALID_TIMER;

  return STATUS_CODE_OK;
}
