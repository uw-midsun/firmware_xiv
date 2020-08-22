#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "spi.h"
#include "status.h"

#define NUM_STORED_CURRENT_READINGS 20
#define CURRENT_SENSE_SPI_PORT SPI_PORT_2

// slightly larger than conversion time of adc
#define CONVERSION_TIME_MS 18

// see current sense on confluence for these values (centimps)
#define DISCHARGE_OVERCURRENT_CA (13000)  // 130 Amps
#define CHARGE_OVERCURRENT_CA (-8160)     // -81.6 Amps

typedef struct CurrentReadings {
  int16_t readings[NUM_STORED_CURRENT_READINGS];
  int16_t average;
} CurrentReadings;

bool current_sense_is_charging();

StatusCode current_sense_init(CurrentReadings *readings, SpiSettings *settings,
                              uint32_t conv_delay);
