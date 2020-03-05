#pragma once

#include "precharge_control.h"
#include "mci_broadcast.h"
#include "mci_output.h"

typedef struct MotorControllerStorage {
  MotorControllerOutputStorage mci_output_storage;
  MotorControllerBroadcastStorage broadcast_storage;
} MotorControllerStorage;
