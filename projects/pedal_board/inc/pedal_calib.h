#pragma once
// Calibration blob for driver controls
#include "calib.h"

#define NUM_SAMPLES 1000

typedef struct ThrottleCalibrationData {
  // When the brake is considered fully unpressed
  int16_t lower_value;
  // When the brake is considered fully pressed
  int16_t upper_value;
} ThrottleCalibrationData;

typedef struct BrakeCalibrationData {
  // When the brake is considered fully unpressed
  int16_t lower_value;
  // When the brake is considered fully pressed
  int16_t upper_value;
} BrakeCalibrationData;

typedef struct PedalCalibBlob {
  ThrottleCalibrationData throttle_calib;
  BrakeCalibrationData brake_calib;
} PedalCalibBlob;
