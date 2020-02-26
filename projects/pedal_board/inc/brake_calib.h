#pragma once
#include "ads1015.h"
#include "pedal_calib.h"
#include "pedal_events.h"

typedef struct BrakeCalibrationStorage {
  int16_t min_reading;
  int16_t max_reading;
  volatile uint32_t sample_counter;
} BrakeCalibrationStorage;

StatusCode brake_calib_init(BrakeCalibrationStorage *storage);

StatusCode brake_calib_sample(BrakeCalibrationStorage *storage, BrakeCalibrationData *data,
                              PedalState state);
