#pragma once

#include "exported_enums.h"
#include "stdint.h"

typedef union FaultReason {
  struct {
    EEConsoleFaultArea area;
    uint8_t reason;
  } fields;
  uint16_t raw;
} FaultReason;
