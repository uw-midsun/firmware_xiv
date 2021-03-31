#pragma once

// Provides all hardware settings and configuration for solar.

#include "can.h"
#include "data_tx.h"
#include "fan_control_solar.h"
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

#define SOLAR_SPI_PORT SPI_PORT_2

const I2CSettings *config_get_i2c1_settings(void);

const I2CSettings *config_get_i2c2_settings(void);

const SpiSettings *config_get_spi_settings(void);

const CanSettings *config_get_can_settings(SolarMpptCount mppt_count);

const GpioAddress *config_get_drv120_enable_pin(void);

const GpioAddress *config_get_drv120_status_pin(void);

const SenseSettings *config_get_sense_settings(void);

const FaultHandlerSettings *config_get_fault_handler_settings(SolarMpptCount mppt_count);

const DataTxSettings *config_get_data_tx_settings(SolarMpptCount mppt_count);

const SenseTemperatureSettings *config_get_sense_temperature_settings(SolarMpptCount mppt_count);

const SenseMcp3427Settings *config_get_sense_mcp3427_settings(SolarMpptCount mppt_count);

const SenseMpptSettings *config_get_sense_mppt_settings(SolarMpptCount mppt_count);

const FaultMonitorSettings *config_get_fault_monitor_settings(SolarMpptCount mppt_count);

const FanControlSettingsSolar *config_get_fan_control_settings(SolarMpptCount mppt_count);
