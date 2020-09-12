#pragma once

// Stores current readings from the ADS1259 in a ring buffer.
// Requires interrupts and soft timers to be initialized.

#include <stdbool.h>
#include <stdint.h>

#include "spi.h"
#include "status.h"

#define NUM_STORED_CURRENT_READINGS 20
#define CURRENT_SENSE_SPI_PORT SPI_PORT_2

// slightly larger than conversion time of adc
#define CONVERSION_TIME_MS 18

// see current sense on confluence for these values (centiamps)
#define DISCHARGE_OVERCURRENT_CA (13000)  // 130 Amps
#define CHARGE_OVERCURRENT_CA (-8160)     // -81.6 Amps

typedef struct CurrentStorage {
  int16_t readings_ring[NUM_STORED_CURRENT_READINGS];
  uint16_t ring_idx;
  int16_t average;
  uint32_t conv_period_ms;
} CurrentStorage;

bool current_sense_is_charging();

StatusCode current_sense_init(CurrentStorage *readings, SpiSettings *settings,
                              uint32_t conv_period_ms);
