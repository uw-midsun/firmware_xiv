#include "voltage_regulator.h"

#include <stddef.h>

#include "gpio.h"
#include "log.h"

#include "interrupt.h"
#include "soft_timer.h"

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  VoltageRegulatorStorage *storage = context;
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&storage->monitor_pin, &state);
  if (state == GPIO_STATE_HIGH && !storage->regulator_on) {
    storage->error_callback(VOLTAGE_REGULATOR_ERROR_ON_WHEN_SHOULD_BE_OFF,
                            storage->error_callback_context);
  } else if (state == GPIO_STATE_LOW && storage->regulator_on) {
    storage->error_callback(VOLTAGE_REGULATOR_ERROR_OFF_WHEN_SHOULD_BE_ON,
                            storage->error_callback_context);
  }
  soft_timer_start_millis(storage->timer_callback_delay_ms, prv_timer_callback, storage, NULL);
}

StatusCode voltage_regulator_init(VoltageRegulatorStorage *storage,
                                  VoltageRegulatorSettings *settings) {
  if (settings == NULL || storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->enable_pin = settings->enable_pin;
  storage->monitor_pin = settings->monitor_pin;
  storage->timer_callback_delay_ms = settings->timer_callback_delay_ms;
  storage->error_callback = settings->error_callback;
  storage->error_callback_context = settings->error_callback_context;
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;

  return soft_timer_start_millis(storage->timer_callback_delay_ms, prv_timer_callback, storage,
                                 NULL);
}

StatusCode voltage_regulator_set_enabled(VoltageRegulatorStorage *storage, bool enable) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->regulator_on = enable;
  return gpio_set_state(&storage->monitor_pin, enable ? GPIO_STATE_HIGH : GPIO_STATE_LOW);
}

void voltage_regulator_stop(VoltageRegulatorStorage *storage) {
  soft_timer_cancel(storage->timer_id);

  // make sure calling stop twice doesn't cancel an unrelated timer
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
}
