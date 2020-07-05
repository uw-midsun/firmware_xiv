#pragma once

// Implementation of sense for reading from the MPPTs.
// Requires the event queue, GPIO, and SPI to be initialized. SPI must be initialized in SPI_MODE_3.

#include "solar_boards.h"
#include "spi.h"
#include "status.h"

typedef struct SenseMpptSettings {
  SolarMpptCount mppt_count;
  SpiPort spi_port;
} SenseMpptSettings;

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_mppt_init(SenseMpptSettings *settings);
