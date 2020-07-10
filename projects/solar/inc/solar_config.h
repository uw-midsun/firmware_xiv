#pragma once

// Holds all hardware settings and configuration for solar.

#include "sense_current.h"
#include "sense_temperature.h"
#include "solar_boards.h"
#include "status.h"

// after this many MCP3427 faults in a row, we raise a fault event
#define MAX_CONSECUTIVE_MCP3427_FAULTS 3

extern const SenseCurrentSettings sense_current_settings;

StatusCode config_get_sense_temperature_settings(SolarMpptCount mppt_count,
                                                 SenseTemperatureSettings *settings);
