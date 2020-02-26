#include "throttle_calib.h"
#include "throttle_data.h"
#include "ads1015.h"
#include "pedal_calib.h"
#include "pedal_data.h"
#include "pedal_events.h"

static void prv_callback_channel(Ads1015Channel ads1015, void *context) {
  ThrottleCalibrationStorage *storage = context;
  int16_t reading = 0;
  ads1015_read_raw(get_ads1015_storage(), get_pedal_data_storage()->brake_channel, &reading);

  if (storage->sample_counter < NUM_SAMPLES) {
    storage->sample_counter++;
    storage->min_reading = MIN(storage->min_reading, reading);
    storage->max_reading = MAX(storage->min_reading, reading);
  }
}

StatusCode throttle_calib_init(ThrottleCalibrationStorage *storage) {
  // not too sure what this does
  memset(storage, 0, sizeof(*storage));
  return STATUS_CODE_OK;
}

// remember that throttle has 2 channels but we currently just use 1
StatusCode throttle_calib_sample(ThrottleCalibrationStorage *storage, ThrottleCalibrationData *data, PedalState state) {
  // Disables channel
  ads1015_configure_channel(get_ads1015_storage(), get_pedal_data_storage()->throttle_channel1, false,
                            NULL, NULL);
  storage->sample_counter = 0;
  storage->min_reading = INT16_MAX;
  storage->max_reading = INT16_MIN;

  ads1015_configure_channel(get_ads1015_storage(), get_pedal_data_storage()->throttle_channel1, true,
                            prv_callback_channel, data);
  while (storage->sample_counter < NUM_SAMPLES) {
    wait();
  }
  if (state == PEDAL_PRESSED) {
    data->lower_value = (storage->min_reading + storage->max_reading) / 2;
  } else {
    data->upper_value = (storage->min_reading + storage->max_reading) / 2;
  }
  return STATUS_CODE_OK;
}
