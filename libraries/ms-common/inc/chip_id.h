#pragma once

#include <stdint.h>

typedef struct ChipId {
  uint32_t id[3];
} ChipId;

ChipId chip_id_get(void);
