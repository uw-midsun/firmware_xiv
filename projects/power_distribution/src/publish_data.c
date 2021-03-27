#include "publish_data.h"

#include <stdint.h>

#include "log.h"
#include "misc.h"
#include "output.h"
#include "soft_timer.h"
#include "status.h"

#define NUM_MSG_PER_ITERATION 8
#define ITERATION_WAIT_MS 10

static PublishDataConfig *s_config = { 0 };
static uint16_t *s_current_measurements = NULL;

StatusCode power_distribution_publish_data_init(PublishDataConfig *config) {
  if (config->transmitter == NULL || config->outputs_to_publish == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // check that each output we're to publish is valid to avoid segfaults
  for (uint16_t c = 0; c < config->num_outputs_to_publish; c++) {
    if (config->outputs_to_publish[c] >= NUM_OUTPUTS) {
      return status_code(STATUS_CODE_INVALID_ARGS);
    }
  }

  s_config = config;
  return STATUS_CODE_OK;
}

static void prv_partially_publish(SoftTimerId timer_id, void *context) {
  if (s_current_measurements == NULL) {
    // guard against possible segfaults
    return;
  }

  // We use a uintptr_t for the index so we can pass it through the context
  uintptr_t index = (uintptr_t)context;

  uint16_t transmit_up_to = MIN(index + NUM_MSG_PER_ITERATION, s_config->num_outputs_to_publish);
  while (index < transmit_up_to) {
    Output current_id = s_config->outputs_to_publish[index];
    StatusCode tx_code = s_config->transmitter(current_id, s_current_measurements[current_id]);
    if (!status_ok(tx_code)) {
      LOG_WARN("Failed to TX current %d: status code %d\n", current_id, tx_code);
    }
    index++;
  }

  if (index < s_config->num_outputs_to_publish) {
    StatusCode code =
        soft_timer_start_millis(ITERATION_WAIT_MS, prv_partially_publish, (void *)index, NULL);
    if (!status_ok(code)) {
      LOG_WARN("Failed to start publish_data soft timer! status code: %d\n", code);
    }
  }
}

StatusCode power_distribution_publish_data_publish(uint16_t current_measurements[NUM_OUTPUTS]) {
  if (s_config->outputs_to_publish == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  s_current_measurements = current_measurements;
  uintptr_t index = 0;
  prv_partially_publish(SOFT_TIMER_INVALID_TIMER, (void *)index);
  return STATUS_CODE_OK;
}
