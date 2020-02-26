#pragma once
#include "ads1015.h"
#include "pedal_data.h"

typedef struct ThrottleCalibrationData {
  // When the brake is considered fully unpressed
  int16_t lower_value;
  // When the brake is considered fully pressed
  int16_t upper_value;
} ThrottleCalibrationData;

StatusCode get_throttle_data(PedalDataStorage *storage, int16_t *position);
