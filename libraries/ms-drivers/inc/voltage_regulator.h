#pragma once
// Driver for the voltage regulator
// When hardware needs 5V power, use this 5V regulator
#include "gpio.h"
#include "gpio_it.h"
enum VoltageRegulatorError { ON_WHEN_SHOULD_BE_OFF, OFF_WHEN_SHOULD_BE_ON };

typedef void (*VoltageRegulatorErrorCallback)(void *context, VoltageRegulatorError error);

typedef struct {
  bool regulator_on;
  GpioAddress enable_pin;
  GpioAddress monitor_pin;
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
