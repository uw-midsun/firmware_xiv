#pragma once
// Calibration blob for driver controls
#include "calib.h"
#include "brake_data.h"
#include "throttle_data.h"

#define NUM_SAMPLES 1000

typedef struct PedalCalibBlob {
  ThrottleCalibrationData throttle_calib;
  BrakeCalibrationData brake_calib;
} PedalCalibBlob;