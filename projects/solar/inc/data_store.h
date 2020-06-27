#pragma once

// Stores all data collected by solar.
// Requires the event queue to be initialized.
// Note that on startup, all data will be garbage, so data consumers must only read data after
// receiving a DATA_READY_EVENT.

#include <stdbool.h>
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

  // The CR bits on the MPPTs: we don't know what they are, but let's keep track of them for now
  // Value of the data points will be 0 or 1
  DATA_POINT_CR_BIT_1,
  DATA_POINT_CR_BIT_2,
  DATA_POINT_CR_BIT_3,
  DATA_POINT_CR_BIT_4,
  DATA_POINT_CR_BIT_5,
  DATA_POINT_CR_BIT_6,

  NUM_DATA_POINTS,
} DataPoint;

// Initialize the data store. Reset so that all data points are not set.
StatusCode data_store_init(void);

// Overwrites the value of the data point with |value|.
StatusCode data_store_set(DataPoint data_point, uint16_t value);

// Call this when you're done a session of calling |data_store_set| and you want data consumers
// to be notified. Raises a DATA_READY_EVENT. Every data point should have been overwritten from
// its initial garbage when this is called.
StatusCode data_store_done(void);

// Puts the value of the data point in |value|. Warning: if the data point is not set in the store,
// this will put garbage in |value|. Check |data_store_get_is_set| before every call.
StatusCode data_store_get(DataPoint data_point, uint16_t *value);

// Puts whether the data point is set in the data store in |is_set|.
StatusCode data_store_get_is_set(DataPoint data_point, bool *is_set);
