#include "exported_enums.h"
#include "log.h"
#include "pedal_calib.h"
#include "pedal_events.h"
#include "pedal_shared_resources_provider.h"
#include "soft_timer.h"

StatusCode get_brake_data(int16_t *position) {
  status_ok_or_return(ads1015_read_raw(get_ads1015_storage(), BRAKE_CHANNEL, position));
  PedalCalibBlob *calib_blob = get_pedal_calib_blob();
  int32_t range = calib_blob->brake_calib.upper_value - calib_blob->brake_calib.lower_value;
  int32_t position_upscaled = (int32_t)*position * EE_PEDAL_VALUE_DENOMINATOR;
  position_upscaled -= calib_blob->brake_calib.lower_value * EE_PEDAL_VALUE_DENOMINATOR;
  position_upscaled *= 100;
  // just to check
  if (range != 0) {
    position_upscaled /= range;
    *position = (int16_t)position_upscaled;
  }
  return STATUS_CODE_OK;
}
