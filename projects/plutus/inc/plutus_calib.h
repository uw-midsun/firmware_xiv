#pragma once
// Calibration blob for plutus
#include "calib.h"
#include "current_sense.h"

typedef struct PlutusCalibBlob {
  CurrentSenseCalibrationData current_calib;
} PlutusCalibBlob;
