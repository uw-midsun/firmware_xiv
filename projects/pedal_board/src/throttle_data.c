#include "ads1015.h"
#include "brake_data.h"
#include "event_queue.h"
#include "exported_enums.h"
#include "fsm.h"
#include "log.h"
#include "pedal_data.h"
#include "pedal_data_tx.h"
#include "pedal_events.h"
#include "soft_timer.h"

StatusCode get_throttle_data(PedalDataTxStorage *storage, int16_t *position) {
  // throttle actually uses 2 channels. may configure later
  status_ok_or_return(
      ads1015_read_raw(get_ads1015_storage(), storage->throttle_channel2, position));
  PedalCalibBlob *calib_blob = get_pedal_calib_blob();
  int32_t range = calib_blob->throttle_calib.upper_value - calib_blob->throttle_calib.lower_value;
  int32_t position_upscaled = (int32_t)*position * EE_PEDAL_MULTIPLYER;LOG_DEBUG("%d\n", position_upscaled);
  position_upscaled -= calib_blob->throttle_calib.lower_value * EE_PEDAL_MULTIPLYER;
  position_upscaled *= 100;
  // just to check
  
  if (range != 0) {
    position_upscaled /= range;
    *position = (int16_t)position_upscaled;
  }
  return STATUS_CODE_OK;
}
