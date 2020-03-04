#include "pedal_shared_resources_provider.h"
#include "ads1015.h"
#include "log.h"
#include "pedal_calib.h"
#include "string.h"

static Ads1015Storage *s_ads1015_storage;
static PedalCalibBlob *s_pedal_calib_blob;

// perhaps also set the #define upper and lower here for calib
StatusCode pedal_resources_init(Ads1015Storage *storage, PedalCalibBlob *calib_blob) {
  s_ads1015_storage = storage;

  // not too sure what this does
  // memset(calib_blob, 0, sizeof(*calib_blob));
  s_pedal_calib_blob = calib_blob;
  LOG_DEBUG("THROTTLE UPPER: %d \n", s_pedal_calib_blob->throttle_calib.upper_value);
  LOG_DEBUG("THROTTLE LOWER: %d \n", s_pedal_calib_blob->throttle_calib.lower_value);
  LOG_DEBUG("BRAKE UPPER: %d \n", s_pedal_calib_blob->brake_calib.upper_value);
  LOG_DEBUG("BRAKE UPPER: %d \n", s_pedal_calib_blob->brake_calib.lower_value);

  // Throttle Channel, there's 2 but we only use 1 right now
  status_ok_or_return(
      ads1015_configure_channel(s_ads1015_storage, THROTTLE_CHANNEL, true, NULL, NULL));
  // brake channel
  status_ok_or_return(
      ads1015_configure_channel(s_ads1015_storage, BRAKE_CHANNEL, true, NULL, NULL));
  return STATUS_CODE_OK;
}

Ads1015Storage *get_ads1015_storage() {
  return s_ads1015_storage;
}

PedalCalibBlob *get_pedal_calib_blob() {
  return s_pedal_calib_blob;
}
