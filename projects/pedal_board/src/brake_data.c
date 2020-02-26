#include "log.h"
#include "pedal_data.h"
#include "pedal_events.h"
#include "soft_timer.h"

// NEED TO CHANGE!!!!!!
#define UPPER_BRAKE_VALUE 1273
#define LOWER_BRAKE_VALUE 295
#define MULTIPLYER 4096

StatusCode get_brake_data(PedalDataStorage *storage, int16_t *position) {
  status_ok_or_return(ads1015_read_raw(get_ads1015_storage(), storage->brake_channel, position));
  int32_t range = UPPER_BRAKE_VALUE - LOWER_BRAKE_VALUE;
  int32_t position_upscaled = (int32_t)*position * MULTIPLYER;
  position_upscaled -= LOWER_BRAKE_VALUE * MULTIPLYER;
  position_upscaled *= 100;
  position_upscaled /= range * MULTIPLYER;
  *position = (int16_t)position_upscaled;
  return STATUS_CODE_OK;
}
