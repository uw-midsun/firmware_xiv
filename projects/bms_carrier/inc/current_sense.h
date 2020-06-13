#pragma once

#include <stdint.h>

#include "spi.h"
#include "status.h"

#define NUM_STORED_CURRENT_READINGS 20

typedef struct CurrentStorage {
  int16_t readings[NUM_STORED_CURRENT_READINGS];
} CurrentStorage;

StatusCode current_sense_init(SpiSettings *settings, CurrentStorage *storage);
