#pragma once
#include "ads1015.h"
#include "pedal_calib.h"
#include "pedal_events.h"

typedef struct ThrottleCalibrationStorage {
  int16_t min_reading;
  int16_t max_reading;
  volatile uint32_t sample_counter;
} ThrottleCalibrationStorage;

StatusCode throttle_calib_init(ThrottleCalibrationStorage *storage);

StatusCode throttle_calib_sample(Ads1015Storage *ads1015_storage,
                                 ThrottleCalibrationStorage *storage, ThrottleCalibrationData *data,
                                 PedalState state);
