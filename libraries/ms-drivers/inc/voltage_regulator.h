#pragma once
// Driver for the voltage regulator
// Use this 5V regulator driver when the hardware needs 5v power.
// Requires interrupts, soft timers, and gpio to be initialized.

// voltage_regulator_init initializes the regulator with the
// provided settings and starts the softimer.

#include "gpio.h"
#include "soft_timer.h"

typedef enum {
  VOLTAGE_REGULATOR_ERROR_ON_WHEN_SHOULD_BE_OFF,
  VOLTAGE_REGULATOR_ERROR_OFF_WHEN_SHOULD_BE_ON,
  NUM_VOLTAGE_REGULATOR_ERRORS
} VoltageRegulatorError;

typedef void (*VoltageRegulatorErrorCallback)(VoltageRegulatorError error, void *context);

typedef struct {
  bool regulator_on;
  GpioAddress enable_pin;
  GpioAddress monitor_pin;
  SoftTimerId timer_id;
  uint32_t timer_callback_delay;
  VoltageRegulatorErrorCallback error_callback;
  void *error_callback_context;
} VoltageRegulatorStorage;

typedef struct {
  GpioAddress enable_pin;
  GpioAddress monitor_pin;
  uint32_t timer_callback_delay;
  VoltageRegulatorErrorCallback error_callback;
  void *error_callback_context;
} VoltageRegulatorSettings;

// initializes the regulator with provided storage and settings
// and starts the soft timer
StatusCode voltage_regulator_init(VoltageRegulatorStorage *storage,
                                  VoltageRegulatorSettings *settings);

// enables or disbales the voltage regulator
StatusCode voltage_regulator_set_enabled(VoltageRegulatorStorage *storage, bool enable);

// function used mostly for test use to stop the softtimer
void voltage_regulator_stop(VoltageRegulatorStorage *storage);
