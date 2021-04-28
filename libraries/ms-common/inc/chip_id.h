#pragma once
// Module for accessing hardware chip id.
// Chip id should stay constant for each controller board.
// Unique device identifier base address from section 33.1 of the specific device
// datasheet.
//
// Emulates x86 system by accessing process id instead of chip id.

#include <stdint.h>

typedef struct ChipId {
  uint32_t id[3];
} ChipId;

ChipId chip_id_get(void);
