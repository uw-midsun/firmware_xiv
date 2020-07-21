#pragma once

// Stores all data collected by solar.
// Requires the event queue to be initialized.
// Note that on startup, all data will be garbage, so data consumers must only read data after
// receiving a DATA_READY_EVENT and checking that each data point is set.

#include <stdbool.h>
#include <stdint.h>

#include "solar_boards.h"
#include "status.h"

#define INVALID_DATA_POINT NUM_DATA_POINTS

// Use these macros to build data points associated with the nth MPPT.
#define DATA_POINT_VOLTAGE(n) NTH_DATA_POINT_CHECK_IMPL(DATA_POINT_TYPE_VOLTAGE, (n))
#define DATA_POINT_TEMPERATURE(n) NTH_DATA_POINT_CHECK_IMPL(DATA_POINT_TYPE_TEMPERATURE, (n))
#define DATA_POINT_MPPT_VOLTAGE(n) NTH_DATA_POINT_CHECK_IMPL(DATA_POINT_TYPE_MPPT_VOLTAGE, (n))
#define DATA_POINT_MPPT_CURRENT(n) NTH_DATA_POINT_CHECK_IMPL(DATA_POINT_TYPE_MPPT_CURRENT, (n))
#define DATA_POINT_MPPT_PWM(n) NTH_DATA_POINT_CHECK_IMPL(DATA_POINT_TYPE_MPPT_PWM, (n))
#define DATA_POINT_CR_BIT(n) NTH_DATA_POINT_CHECK_IMPL(DATA_POINT_TYPE_CR_BIT, (n))

// Special case: DATA_POINT_TYPE_CURRENT isn't associated with an MPPT.
#define DATA_POINT_CURRENT NTH_DATA_POINT_IMPL(DATA_POINT_TYPE_CURRENT, 0)

// The total number of data points.
#define NUM_DATA_POINTS NTH_DATA_POINT_IMPL(NUM_DATA_POINT_TYPES, 0)

// Implementation for the above macros, do not call directly.
// We store data points as uint8s, the MPPT number is the least significant MAX_MPPT_BIT_WIDTH bits
// and the type is the just above that. This scheme uses at most double the space necessary,
// plus the few bytes wasted for DATA_POINT_CURRENT, which only uses the first allocated data point.
#define NTH_DATA_POINT_CHECK_IMPL(type, n) \
  ((n) >= MAX_SOLAR_BOARD_MPPTS ? INVALID_DATA_POINT : NTH_DATA_POINT_IMPL(type, n))
#define NTH_DATA_POINT_IMPL(type, n) (((type) << MAX_MPPT_BIT_WIDTH) | (n))

typedef enum {
  DATA_POINT_TYPE_VOLTAGE = 0,
  DATA_POINT_TYPE_CURRENT,
  DATA_POINT_TYPE_TEMPERATURE,
  DATA_POINT_TYPE_MPPT_VOLTAGE,
  DATA_POINT_TYPE_MPPT_CURRENT,
  DATA_POINT_TYPE_MPPT_PWM,
  DATA_POINT_TYPE_CR_BIT,
  NUM_DATA_POINT_TYPES,
} DataPointType;

typedef uint8_t DataPoint;

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
