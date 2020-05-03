#include "current_calibration.h"

#include <string.h>

#include "critical_section.h"
#include "wait.h"

static void prv_callback(int32_t *value, void *context) {
  CurrentCalibrationStorage *storage = (CurrentCalibrationStorage *)context;

  if (storage->samples < CURRENT_CALIBRATION_SAMPLES) {
    storage->voltage += *value;
    storage->samples++;
  }
}

StatusCode current_calibration_init(CurrentCalibrationStorage *storage,
                                    LtcAdcSettings *adc_settings) {
  if (storage == NULL || adc_settings == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  storage->settings = adc_settings;
  storage->samples = 0;
  storage->voltage = 0;

  status_ok_or_return(ltc_adc_init(&storage->adc_storage, storage->settings));

  return STATUS_CODE_OK;
}

StatusCode current_calibration_sample_point(CurrentCalibrationStorage *storage,
                                            CurrentSenseValue *point, int32_t current) {
  if (storage == NULL || point == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  status_ok_or_return(ltc_adc_register_callback(&storage->adc_storage, prv_callback, storage));

  while (storage->samples < CURRENT_CALIBRATION_SAMPLES) {
    wait();
  }

  // Disable callback
  ltc_adc_register_callback(&storage->adc_storage, NULL, storage);

  storage->voltage /= CURRENT_CALIBRATION_SAMPLES;

  point->voltage = storage->voltage;
  point->current = current;

  storage->samples = 0;
  storage->voltage = 0;

  return STATUS_CODE_OK;
}
