#pragma once

// Provides all hardware settings and configuration for solar.

#include "can.h"
#include "fault_handler.h"
#include "fault_monitor.h"
#include "gpio.h"
#include "i2c.h"
#include "relay_fsm.h"
#include "sense.h"
#include "sense_mcp3427.h"
#include "sense_mppt.h"
#include "sense_temperature.h"
#include "solar_boards.h"
#include "spi.h"
#include "status.h"

extern const I2CSettings i2c1_settings;
extern const I2CSettings i2c2_settings;
extern const SpiSettings spi_settings;

extern const CanSettings can_settings;

extern const GpioAddress drv120_relay_pin;

extern const SenseSettings sense_settings;

extern const FaultHandlerSettings fault_handler_settings;

StatusCode config_get_sense_temperature_settings(SolarMpptCount mppt_count,
                                                 SenseTemperatureSettings *settings);

StatusCode config_get_sense_mcp3427_settings(SolarMpptCount mppt_count,
                                             SenseMcp3427Settings *settings);

StatusCode config_get_sense_mppt_settings(SolarMpptCount mppt_count, SenseMpptSettings *settings);

StatusCode config_get_fault_monitor_settings(SolarMpptCount mppt_count,
                                             FaultMonitorSettings *settings);
