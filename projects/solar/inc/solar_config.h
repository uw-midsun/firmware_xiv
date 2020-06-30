#pragma once

// Holds all hardware settings and configuration for solar.

#include "sense_current.h"
#include "sense_mcp3427.h"

// after this many MCP3427 faults in a row, we raise a fault event
#define MAX_CONSECUTIVE_MCP3427_FAULTS 3

typedef enum SolarBoardType {
  SOLAR_BOARD_TYPE_5_MPPT = 0,
  SOLAR_BOARD_TYPE_6_MPPT,
  NUM_SOLAR_BOARD_TYPES,
} SolarBoardType;

extern const SenseCurrentSettings sense_current_settings;

StatusCode config_get_sense_mcp3427_settings(SolarBoardType board_type,
                                             SenseMcp3427Settings *settings);
