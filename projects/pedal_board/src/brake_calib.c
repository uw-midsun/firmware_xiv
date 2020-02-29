#include "brake_calib.h"
#include "ads1015.h"
#include "brake_data.h"
#include "pedal_calib.h"
#include "pedal_data.h"
#include "pedal_events.h"
#include "string.h"
#include "wait.h"

int32_t average_value;

static void prv_callback_channel(Ads1015Channel ads1015, void *context) {
  BrakeCalibrationStorage *storage = context;
  int16_t reading = 0;
  ads1015_read_raw(get_ads1015_storage(), get_pedal_data_storage()->brake_channel, &reading);

  if (storage->sample_counter < NUM_SAMPLES) {
    storage->sample_counter++;
    average_value += (int32_t)reading;
    storage->min_reading = MIN(storage->min_reading, reading);
    storage->max_reading = MAX(storage->min_reading, reading);
  }
}

StatusCode brake_calib_init(BrakeCalibrationStorage *storage) {
  // not too sure what this does
  memset(storage, 0, sizeof(*storage));
  return STATUS_CODE_OK;
}

StatusCode brake_calib_sample(BrakeCalibrationStorage *storage, BrakeCalibrationData *data,
                              PedalState state) {
  average_value = 0;
  // Disables channel
  ads1015_configure_channel(get_ads1015_storage(), get_pedal_data_storage()->brake_channel, false,
                            NULL, NULL);
  storage->sample_counter = 0;
  storage->min_reading = INT16_MAX;
  storage->max_reading = INT16_MIN;

  ads1015_configure_channel(get_ads1015_storage(), get_pedal_data_storage()->brake_channel, true,
                            prv_callback_channel, data);
  while (storage->sample_counter < NUM_SAMPLES) {
    wait();
  }

  if (state == PEDAL_PRESSED) {
    data->lower_value = average_value / 1000;
  } else {
    data->upper_value = average_value / 1000;
  }
  return STATUS_CODE_OK;
}
