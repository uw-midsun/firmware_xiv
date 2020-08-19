#pragma once

// Provides all hardware settings and configuration for solar.

#include "fault_monitor.h"
#include "sense_mcp3427.h"
#include "sense_mppt.h"
#include "sense_temperature.h"
#include "solar_boards.h"
#include "solar_fsm.h"
#include "status.h"

extern const SolarFsmSettings solar_fsm_settings;

StatusCode config_get_sense_temperature_settings(SolarMpptCount mppt_count,
                                                 SenseTemperatureSettings *settings);

StatusCode config_get_sense_mcp3427_settings(SolarMpptCount mppt_count,
                                             SenseMcp3427Settings *settings);

StatusCode config_get_sense_mppt_settings(SolarMpptCount mppt_count, SenseMpptSettings *settings);

StatusCode config_get_fault_monitor_settings(SolarMpptCount mppt_count,
                                             FaultMonitorSettings *settings);
