#pragma once

// Implementation of sense for reading from the thermistors.
// Requires the event queue, GPIO, sense, and the data store to be initialized.

#include "data_store.h"
#include "gpio.h"
#include "solar_boards.h"
#include "status.h"

#define MAX_THERMISTORS MAX_SOLAR_BOARD_MPPTS

typedef struct SenseTemperatureThermistorConfig {
  DataPoint data_point;
  GpioAddress pin;
} SenseTemperatureThermistorConfig;

typedef struct SenseTemperatureSettings {
  SenseTemperatureThermistorConfig thermistors[MAX_THERMISTORS];
  uint8_t num_thermistors;
} SenseTemperatureSettings;

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_temperature_init(SenseTemperatureSettings *settings);
