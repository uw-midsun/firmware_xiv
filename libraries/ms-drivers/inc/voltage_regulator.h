#pragma once
// Driver for the voltage regulator
// Use this 5V regulator driver when the hardware needs 5v power.
// Requires interrupts, soft timers, and gpio to be initialized.

// voltage_regulator_init initializes the regulator with the
// provided settings and starts the softimer.

#include "gpio.h"
#include "gpio_it.h"
#include "soft_timer.h"

typedef enum {
  VOLTAGE_REGULATOR_ERROR_ON_WHEN_SHOULD_BE_OFF,
  VOLTAGE_REGULATOR_ERROR_OFF_WHEN_SHOULD_BE_ON,
  NUM_VOLTAGE_REGULATOR_ERRORS
} VoltageRegulatorError;

typedef void (*VoltageRegulatorErrorCallback)(void *context, VoltageRegulatorError error);

typedef struct {
  bool regulator_on;
  GpioAddress enable_pin;
  GpioAddress monitor_pin;
  SoftTimerId timer_id;
  VoltageRegulatorErrorCallback error_callback;
  void *error_callback_context;
} VoltageRegulatorStorage;

typedef struct {
  GpioAddress enable_pin;
  GpioAddress monitor_pin;
  VoltageRegulatorErrorCallback error_callback;
  void *error_callback_context;
} VoltageRegulatorSettings;

StatusCode voltage_regulator_init(VoltageRegulatorSettings *settings,
                                  VoltageRegulatorStorage *storage);

StatusCode voltage_regulator_set_enabled(VoltageRegulatorStorage *storage, bool switch_action);

void voltage_regulator_stop(VoltageRegulatorStorage *storage);
