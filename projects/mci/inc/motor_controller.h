#pragma once
#include "gpio.h"

#include "precharge_control.h"

typedef struct MotorControllerStorage {
  PrechargeStorage precharge_storage;
} MotorControllerStorage;
