#pragma once

// Stores all data collected by solar.
// Requires the event queue to be initialized.

#include <stdint.h>
#include "status.h"

typedef enum {
  // Voltages from the voltage sense MCP3427s
  DATA_POINT_VOLTAGE_1 = 0,
  DATA_POINT_VOLTAGE_2,
  DATA_POINT_VOLTAGE_3,
  DATA_POINT_VOLTAGE_4,
  DATA_POINT_VOLTAGE_5,
  DATA_POINT_VOLTAGE_6,
  
  // Current from the current sense MCP3427
  DATA_POINT_CURRENT,
  
  // Temperatures from the thermistors
  DATA_POINT_TEMPERATURE_1,
  DATA_POINT_TEMPERATURE_2,
  DATA_POINT_TEMPERATURE_3,
  DATA_POINT_TEMPERATURE_4,
  DATA_POINT_TEMPERATURE_5,
  DATA_POINT_TEMPERATURE_6,
  
  // MPPT input currents
  DATA_POINT_MPPT_CURRENT_1,
  DATA_POINT_MPPT_CURRENT_2,
  DATA_POINT_MPPT_CURRENT_3,
  DATA_POINT_MPPT_CURRENT_4,
  DATA_POINT_MPPT_CURRENT_5,
  DATA_POINT_MPPT_CURRENT_6,
  
  // MPPT input voltages
  DATA_POINT_MPPT_VOLTAGE_1,
  DATA_POINT_MPPT_VOLTAGE_2,
  DATA_POINT_MPPT_VOLTAGE_3,
  DATA_POINT_MPPT_VOLTAGE_4,
  DATA_POINT_MPPT_VOLTAGE_5,
  DATA_POINT_MPPT_VOLTAGE_6,
  
  // MPPT current PWM duty cycles, out of 1000
  DATA_POINT_MPPT_PWM_1,
  DATA_POINT_MPPT_PWM_2,
  DATA_POINT_MPPT_PWM_3,
  DATA_POINT_MPPT_PWM_4,
  DATA_POINT_MPPT_PWM_5,
  DATA_POINT_MPPT_PWM_6,
  
  NUM_DATA_POINT_POINTS,
} DataPoint;

// Overwrite the value of the data point with |value|.
StatusCode data_store_enter(DataPoint data_point, uint16_t value);

// Call this when you're done a session of calling |data_store_enter| and you want data consumers
// to be notified. Raises a DATA_READY_EVENT.
StatusCode data_store_done(void);

// Put the value of the data point in |value|.
StatusCode data_store_get(DataPoint data_point, uint16_t *value);
