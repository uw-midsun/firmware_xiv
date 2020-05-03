#include "current_sense.h"

#include <string.h>

#include "critical_section.h"
#include "log.h"
#include "status.h"
#include "wait.h"

static void prv_calculate_current(int32_t *value, void *context) {
  CurrentSenseStorage *storage = (CurrentSenseStorage *)context;

  // Update the offset if the flag is set
  if (storage->offset_pending) {
    storage->offset = *value - storage->data.zero_point.voltage;
    storage->offset_pending = false;
  }

  // Formula for calculating calibrated current. Draws slope between given calibrated
  // points, and uses the result as well as the voltage offset to calculate current
  storage->value.voltage = *value;
  storage->value.current = (*value - storage->data.zero_point.voltage - storage->offset) *
                           ((storage->data.max_point.current - storage->data.zero_point.current) /
                            (storage->data.max_point.voltage - storage->data.zero_point.voltage));

  storage->data_valid = true;
  if (storage->callback != NULL) {
    storage->callback(storage->value.current, storage->context);
  }
}

static void prv_fault_handle(void *context) {
  CurrentSenseStorage *storage = (CurrentSenseStorage *)context;

  storage->data_valid = false;
  if (storage->fault_callback != NULL) {
    storage->fault_callback(storage->context);
  }
  return;
}

StatusCode current_sense_init(CurrentSenseStorage *storage, const CurrentSenseCalibrationData *data,
                              const LtcAdcSettings *settings) {
  if (storage == NULL || settings == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  status_ok_or_return(ltc_adc_init(&storage->adc_storage, settings));

  // Store calibration parameters
  storage->data = *data;

  // Reset data and callbacks
  storage->value.voltage = 0;
  storage->value.current = 0;
  storage->data_valid = false;

  storage->offset = 0;
  storage->offset_pending = false;
  storage->callback = NULL;
  storage->context = NULL;

  // Register callbacks
  status_ok_or_return(
      ltc_adc_register_callback(&storage->adc_storage, prv_calculate_current, storage));
  status_ok_or_return(
      ltc_adc_register_fault_callback(&storage->adc_storage, prv_fault_handle, storage));

  return STATUS_CODE_OK;
}

// Register a callback to run when new data is available
StatusCode current_sense_register_callback(CurrentSenseStorage *storage,
                                           CurrentSenseCallback callback,
                                           CurrentSenseFaultCallback fault_callback,
                                           void *context) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  storage->callback = callback;
  storage->context = context;
  storage->fault_callback = fault_callback;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode current_sense_get_value(CurrentSenseStorage *storage, int32_t *current) {
  if (storage == NULL || current == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  if (storage->data_valid == false) {
    return status_code(STATUS_CODE_TIMEOUT);
  }

  *current = storage->value.current;

  return STATUS_CODE_OK;
}

StatusCode current_sense_zero_reset(CurrentSenseStorage *storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_UNINITIALIZED);
  }

  // The next conversion is guaranteed to sample the shunt at 0 amps, so we set an offset flag
  storage->offset_pending = true;

  return STATUS_CODE_OK;
}
