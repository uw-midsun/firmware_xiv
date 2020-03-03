#include "pedal_calib.h"
#include "ads1015.h"
#include "log.h"
#include "pedal_calib.h"
#include "pedal_shared_resources_provider.h"
#include "pedal_events.h"
#include "string.h"
#include "throttle_data.h"
#include "wait.h"

int32_t average_value;
static Ads1015Storage *s_ads1015_storage;

static void prv_callback_channel(Ads1015Channel channel, void *context) {
  PedalCalibrationStorage *storage = context;
  int16_t reading = 0;
  ads1015_read_raw(s_ads1015_storage, channel, &reading);

  if (storage->sample_counter < NUM_SAMPLES) {
    storage->sample_counter++;
    average_value += (int32_t)reading;
    storage->min_reading = MIN(storage->min_reading, reading);
    storage->max_reading = MAX(storage->min_reading, reading);
  }
}

StatusCode pedal_calib_init(PedalCalibrationStorage *storage) {
  // not too sure what this does
  memset(storage, 0, sizeof(*storage));
  return STATUS_CODE_OK;
}

// remember that throttle has 2 channels but we currently just use 1
StatusCode pedal_calib_sample(Ads1015Storage *ads1015_storage,
                                 PedalCalibrationStorage *storage, PedalCalibrationData *data, Ads1015Channel channel,
                                 PedalState state) {
  s_ads1015_storage = ads1015_storage;
  average_value = 0;
  // Disables channel
  ads1015_configure_channel(s_ads1015_storage, channel, false, NULL, NULL);
  storage->sample_counter = 0;
  storage->min_reading = INT16_MAX;
  storage->max_reading = INT16_MIN;

  ads1015_configure_channel(s_ads1015_storage, channel, true, prv_callback_channel,
                            storage);

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
