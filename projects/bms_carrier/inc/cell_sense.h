#pragma once

#include <stdint.h>

#include "spi.h"
#include "status.h"

typedef enum { AFE_0, AFE_1, AFE_2, NUM_AFES } AfeIds;

#define NUM_CELL_MODULES_PER_AFE 6
#define NUM_THERMISTORS_PER_AFE 30

typedef struct AfeStorage {
  uint16_t voltages[NUM_AFES][NUM_CELL_MODULES_PER_AFE];
  uint16_t temps[NUM_AFES][NUM_THERMISTORS_PER_AFE];
} AfeStorage;

StatusCode cell_sense_init(SpiSettings *settings, AfeStorage *storage);
