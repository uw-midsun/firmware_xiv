#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "spi.h"
#include "status.h"

#define NUM_STORED_CURRENT_READINGS 20

typedef struct CurrentReadings {
  int16_t readings[NUM_STORED_CURRENT_READINGS];
} CurrentReadings;

bool current_sense_is_charging();

StatusCode current_sense_init(SpiSettings *settings);
