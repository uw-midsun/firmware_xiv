#pragma once
#include "gpio.h"

#include "precharge_control.h"

typedef struct MotorControllerSettings {
  // MCI Rev5: precharge_control = Pin A9
  GpioAddress precharge_control;
  // Thanks Hardware :angry:
  // MCI Rev5: precharge_control2 = Pin B1
  GpioAddress precharge_control2;
  // MCI Rev5: precharge_monitor = Pin B0
  GpioAddress precharge_monitor;
} MotorControllerSettings;

typedef struct MotorControllerStorage {
  MotorControllerSettings settings;
  PrechargeState precharge_state;
} MotorControllerStorage;
