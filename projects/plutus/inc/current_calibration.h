#pragma once

// LTC 2484 calibration module. Samples at fixed points in order to obtain data
// for two-point current calibration. To use, call current_calibration_sample_point()
// to obtain samples at both a known zero and a known max, which will generate two reference
// points that will allow for the calculation of current from an adc sample

// Requires gpio, interrupts, and soft timers to be initialized

#include <assert.h>
#include "current_sense.h"

// Must be less than 128, or overflow may occur
#define CURRENT_CALIBRATION_SAMPLES 10

static_assert(CURRENT_CALIBRATION_SAMPLES < 128, "Sample limit too large. May cause overflow");

typedef struct {
  LtcAdcStorage adc_storage;
  LtcAdcSettings *settings;
  int32_t voltage;
  volatile uint8_t samples;
} CurrentCalibrationStorage;

// Initialize current calibration
StatusCode current_calibration_init(CurrentCalibrationStorage *storage,
                                    LtcAdcSettings *adc_settings);

// Samples adc readings at the specified current (in uA)  in order to obtain data for
// two-point calibration. Function will block until completion. For optimal results, make
// sure the points are as far apart as possible.
StatusCode current_calibration_sample_point(CurrentCalibrationStorage *storage,
                                            CurrentSenseValue *point, int32_t current);
