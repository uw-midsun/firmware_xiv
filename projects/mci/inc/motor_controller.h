#pragma once

#include "mci_broadcast.h"
#include "mci_output.h"
#include "precharge_control.h"

typedef struct MotorControllerStorage {
  MotorControllerOutputStorage mci_output_storage;
  MotorControllerBroadcastStorage broadcast_storage;
} MotorControllerStorage;
