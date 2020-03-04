#pragma once
#include "ads1015.h"
#include "calib.h"
#include "pedal_calib.h"
#include "pedal_events.h"

#define NUM_SAMPLES 1000

typedef enum {
  PEDAL_PRESSED = 0,
  PEDAL_UNPRESSED,
  NUM_PEDAL_STATES,
} PedalState;

typedef struct PedalCalibrationData {
  // When the pedal is considered fully unpressed
  int16_t lower_value;
  // When the pedal is considered fully pressed
  int16_t upper_value;
} PedalCalibrationData;

typedef struct PedalCalibBlob {
  PedalCalibrationData throttle_calib;
  PedalCalibrationData brake_calib;
} PedalCalibBlob;

typedef struct PedalCalibrationStorage {
  int16_t min_reading;
  int16_t max_reading;
  volatile uint32_t sample_counter;
} PedalCalibrationStorage;

StatusCode pedal_calib_init(PedalCalibrationStorage *storage);

StatusCode pedal_calib_sample(Ads1015Storage *ads1015_storage, PedalCalibrationStorage *storage,
                              PedalCalibrationData *data, Ads1015Channel channel, PedalState state);
