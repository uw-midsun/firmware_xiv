#pragma once
#include "ads1015.h"
#include "pedal_data_tx.h"

typedef struct BrakeCalibrationData {
  // When the brake is considered fully unpressed
  int16_t lower_value;
  // When the brake is considered fully pressed
  int16_t upper_value;
} BrakeCalibrationData;

StatusCode get_brake_data(PedalDataStorage *storage, int16_t *position);
