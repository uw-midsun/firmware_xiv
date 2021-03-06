#pragma once

// Implementation of sense for reading from the thermistors.
// Requires GPIO, ADC, sense, and the data store to be initialized.
// ADC should be initialized in ADC_MODE_SINGLE.

#include "gpio.h"
#include "solar_boards.h"
#include "status.h"

#define MAX_THERMISTORS MAX_SOLAR_BOARD_MPPTS

typedef enum ThermistorType {
  NTC_THERMISTOR = 0,
  RTD_THERMISTOR,
  FAN_CONTROL_THERMISTOR,
  NUM_THERMISTOR_TYPES,
} ThermistorType;

typedef struct ThermistorSettings {
  ThermistorType thermistor_type;
  GpioAddress thermistor_address;
} ThermistorSettings;

typedef struct SenseTemperatureSettings {
  // The associated data point for the pin at index idx is DATA_POINT_TEMPERATURE(idx)
  ThermistorSettings thermistor_settings[MAX_THERMISTORS];
  uint8_t num_thermistors;
} SenseTemperatureSettings;

// Initialize the module and register it with sense. Must be called after |sense_init|.
StatusCode sense_temperature_init(SenseTemperatureSettings *settings);
