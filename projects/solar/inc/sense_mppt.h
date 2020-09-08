#pragma once

// Implementation of sense for reading from the MPPTs. Also checks MPPT statuses for faults.
// Requires the event queue, GPIO, SPI, mppt, sense, the data store, and the fault handler to be
// initialized. SPI must be initialized in SPI_MODE_3.

#include "solar_boards.h"
#include "spi.h"
#include "status.h"

typedef struct SenseMpptSettings {
  SolarMpptCount mppt_count;
  SpiPort spi_port;

  // The factors to multiply the raw values from the MPPTs by to get the needed units.
  float mppt_current_scaling_factor;
  float mppt_vin_scaling_factor;
} SenseMpptSettings;

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_mppt_init(SenseMpptSettings *settings);
