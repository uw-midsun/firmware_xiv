#pragma once

// Provides all hardware settings and configuration for solar.

#include "sense_mcp3427.h"

// after this many MCP3427 faults in a row, we raise a fault event
#define MAX_CONSECUTIVE_MCP3427_FAULTS 3

typedef enum SolarMpptCount {
  SOLAR_BOARD_5_MPPTS = 5,
  SOLAR_BOARD_6_MPPTS = 6,
  MAX_SOLAR_BOARD_MPPTS = 6,
} SolarMpptCount;

StatusCode config_get_sense_mcp3427_settings(SolarMpptCount mppt_count,
                                             SenseMcp3427Settings *settings);
