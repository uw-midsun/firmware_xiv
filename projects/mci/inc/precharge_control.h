#pragma once

#include <stdbool.h>

#include "gpio.h"
#include "status.h"

// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

typedef enum {
  MCI_PRECHARGE_DISCHARGED = 0,
  MCI_PRECHARGE_INCONSISTENT,
  MCI_PRECHARGE_CHARGED
} PrechargeState;

typedef struct PrechargeControlSettings {
  GpioAddress precharge_control;
  GpioAddress precharge_monitor;
  GpioAddress precharge_monitor2;
} PrechargeControlSettings;

typedef struct PrechargeControlStorage {
  PrechargeState state;
  GpioAddress precharge_control;
  GpioAddress precharge_monitor;
  GpioAddress precharge_monitor2;
  bool initialized;
} PrechargeControlStorage;

PrechargeState get_precharge_state();

StatusCode precharge_control_init(const PrechargeControlSettings *settings);

PrechargeControlStorage *test_get_storage(void);
