#pragma once
#include "status.h"

#include "motor_controller.h"

// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

typedef enum { MCI_PRECHARGE_DISCHARGED = 0, MCI_PRECHARGE_CHARGED } PrechargeState;

typedef struct PrechargeControlSettings {
  GpioAddress precharge_control;
  GpioAddress precharge_control2;
  GpioAddress precharge_monitor;
} PrechargeControlSettings;

typedef struct PrechargeControlStorage {
  PrechargeState state;
  GpioAddress precharge_control;
  GpioAddress precharge_control2;
  GpioAddress precharge_monitor;
  bool initialized;
} PrechargeControlStorage;

PrechargeState get_precharge_state();

StatusCode precharge_control_init(const PrechargeControlSettings *settings);

PrechargeControlStorage *test_get_storage(void);
