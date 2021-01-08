#include "voltage_regulator.h"

#include <stddef.h>
#include <string.h>

#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"

static prv_interrupt_callback(GpioAddress monitor_pin, void *context) {
  VoltageRegulatorStorage *storage = context;
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(&monitor_pin, &state);
  if ((state == GPIO_STATE_HIGH && !(storage->regulator_on)) ||
      (state == GPIO_STATE_LOW && (storage->regulator_on))) {
    storage->error_callback();
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

  InterruptSettings interrupt_settings =
      {
        .type = INTERRUPT_TYPE_INTERRUPT,
        .priority = INTERRUPT_PRIORITY_NORMAL,
      }

  gpio_it_register_interrupt(
      &(*settings->monitor_pin), &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
      prv_interrupt_callback(storage->monitor_pin, storage), settings->error_callback_context);

  return STATUS_CODE_OK;
}

StatusCode voltage_regulator_set_enabled(VoltageRegulatorStorage *storage, bool switch_action) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (!(storage->regulator_on) && switch_action) {
    GpioState state = GPIO_STATE_HIGH;
    gpio_get_state(&monitor_pin, &state);
  } else if (storage->regulator_on && !switch_action) {
    GpioState state = GPIO_STATE_LOW;
    gpio_get_state(&monitor_pin, &state);
  }
  return STATUS_CODE_OK;
}
