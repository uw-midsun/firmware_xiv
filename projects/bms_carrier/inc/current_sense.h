#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "spi.h"
#include "status.h"

#define NUM_STORED_CURRENT_READINGS 20
#define CURRENT_SENSE_SPI_PORT SPI_PORT_2

typedef struct CurrentReadings {
  int16_t readings[NUM_STORED_CURRENT_READINGS];
  int16_t average;
} CurrentReadings;

bool current_sense_is_charging(readings);

StatusCode current_sense_init(CurrentReadings *readings, SpiSettings *settings);
