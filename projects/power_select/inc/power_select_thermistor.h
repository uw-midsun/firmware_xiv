#pragma once

// Handles calculations from thermistors (copied from old power_selection code)

double resistance_to_temp(double resistance);

#define POWER_SELECT_THERMISTOR_VREF 3000.0f

// Determined experimentally through testing
#define POWER_SELECT_VREF_SCALING 10.0f

#define POWER_SELECT_THERMISTOR_VREF_SCALED \
  (POWER_SELECT_THERMISTOR_VREF * POWER_SELECT_VREF_SCALING)

#define MV_TO_V 1000.0f

#define POWER_SELECT_THERMISTOR_RESISTOR 10000

#define voltage_to_res(v) \
  POWER_SELECT_THERMISTOR_VREF_SCALED / (double)((v) / MV_TO_V) - POWER_SELECT_THERMISTOR_RESISTOR
