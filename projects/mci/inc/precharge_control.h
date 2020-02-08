#pragma once
#include "status.h"

#include "motor_controller.h"

// Requires GPIO to be initialized
// Requires GPIO interrupts to be initialized

typedef enum { MCI_PRECHARGE_DISCHARGED = 0, MCI_PRECHARGE_CHARGED } PrechargeState;

typedef struct PrechargeStorage {
  PrechargeState state;
  // MCI Rev5: precharge_control = Pin A9
  GpioAddress precharge_control;
  // Thanks Hardware :angry:
  // MCI Rev5: precharge_control2 = Pin B1
  GpioAddress precharge_control2;
  // MCI Rev5: precharge_monitor = Pin B0
  GpioAddress precharge_monitor;
} PrechargeStorage;

GpioState get_precharge_state(void *context);

StatusCode precharge_control_init(MotorControllerStorage *context);
