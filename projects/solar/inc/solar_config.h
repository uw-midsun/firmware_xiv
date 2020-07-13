#pragma once

// Provides all hardware settings and configuration for solar.

#include "sense_mcp3427.h"
#include "solar_boards.h"

StatusCode config_get_sense_mcp3427_settings(SolarMpptCount mppt_count,
                                             SenseMcp3427Settings *settings);
