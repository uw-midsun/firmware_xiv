#pragma once

// Current sense module for the LTC2484 ADC.

// Requires gpio, interrupts, and soft timers to be initialized

// In the event of an fault in sampling data from the adc (i.e. a disconnection occurs), fault
// callbacks can be defined to handle erroneous behaviour

#include "ltc_adc.h"
#include "status.h"

// Runs when a valid sample has been taken
typedef void (*CurrentSenseCallback)(int32_t current, void *context);

// Runs when an adc fault has occured
typedef void (*CurrentSenseFaultCallback)(void *context);

// Voltage-current point used for calculations
typedef struct {
  int32_t voltage;
  int32_t current;
} CurrentSenseValue;

// Line data for two-point calibration
typedef struct {
  CurrentSenseValue zero_point;
  CurrentSenseValue max_point;
} CurrentSenseCalibrationData;

typedef struct {
  LtcAdcStorage adc_storage;
  CurrentSenseCalibrationData data;
  CurrentSenseValue value;
  int32_t offset;
  CurrentSenseCallback callback;
  CurrentSenseFaultCallback fault_callback;
  void *context;
  bool offset_pending;
  bool data_valid;
} CurrentSenseStorage;

// Initialize the current sense module. Requires |data| to be calibrated beforehand.
// |settings| does not need to persist
StatusCode current_sense_init(CurrentSenseStorage *storage, const CurrentSenseCalibrationData *data,
                              const LtcAdcSettings *settings);

// Register a callback to run after each sample, along with a fault callback to run when
// an adc fault occurs
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback,
                                           CurrentSenseFaultCallback fault_callback, void *context);

// Returns the most recent current sample in uA. The status code returned will indcate if
// the data was obtained from a valid conversion sample
StatusCode current_sense_get_value(CurrentSenseStorage *storage, int32_t *current);

// Because the zero-point changes each time the chip is reset, this function can be used to
// ensure the calibration data does not skew the output due to an offset mismatch
StatusCode current_sense_zero_reset(CurrentSenseStorage *storage);
