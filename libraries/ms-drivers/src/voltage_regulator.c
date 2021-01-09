#include "voltage_regulator.h"

#include <stddef.h>
#include <string.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "soft_timer.h"
#define TIME_INTERVAL 2000

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  VoltageRegulatorStorage *storage = context;
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&(storage->monitor_pin), &state);
  if (state == GPIO_STATE_HIGH && !(storage->regulator_on)) {
    storage->error_callback(storage->error_callback_context,
                            VOLTAGE_REGULATOR_ERROR_ON_WHEN_SHOULD_BE_OFF);
  } else if (state == GPIO_STATE_LOW && (storage->regulator_on)) {
    storage->error_callback(storage->error_callback_context,
                            VOLTAGE_REGULATOR_ERROR_OFF_WHEN_SHOULD_BE_ON);
  }
}

StatusCode voltage_regulator_init(VoltageRegulatorSettings *settings,
                                  VoltageRegulatorStorage *storage) {
  if (settings == NULL || storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  storage->enable_pin = settings->enable_pin;
  storage->monitor_pin = settings->monitor_pin;
  storage->error_callback = settings->error_callback;
  storage->error_callback_context = settings->error_callback_context;
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
  soft_timer_start_millis(TIME_INTERVAL, prv_timer_callback, storage, NULL);
  return STATUS_CODE_OK;
}

StatusCode voltage_regulator_set_enabled(VoltageRegulatorStorage *storage, bool switch_action) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (!(storage->regulator_on) && switch_action) {
    GpioState state = GPIO_STATE_HIGH;
    gpio_set_state(&(storage->monitor_pin), state);
  } else if (storage->regulator_on && !switch_action) {
    GpioState state = GPIO_STATE_LOW;
    gpio_set_state(&(storage->monitor_pin), state);
  }
  return STATUS_CODE_OK;
}

void voltage_regulator_stop(VoltageRegulatorStorage *storage) {
  soft_timer_cancel(storage->timer_id);

  // make sure calling stop twice doesn't cancel an unrelated timer
  storage->timer_id = SOFT_TIMER_INVALID_TIMER;
}
