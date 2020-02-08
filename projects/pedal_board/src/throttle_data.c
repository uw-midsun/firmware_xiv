#include "ads1015.h"
#include "brake_data.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "pedal_data_tx.h"
#include "pedal_events.h"
#include "soft_timer.h"

#define UPPER_THROTTLE_VALUE 1273
#define LOWER_THROTTLE_VALUE 295
#define MULTIPLYER 4096

StatusCode get_throttle_data(PedalDataStorage *storage, int16_t *position) {
  // throttle actually uses 2 channels. may configure later
  status_ok_or_return(ads1015_read_raw(storage->storage, storage->throttle_channel2, position));
  float percent = UPPER_THROTTLE_VALUE - LOWER_THROTTLE_VALUE;
  float temp = *position;
  temp -= LOWER_THROTTLE_VALUE;
  temp *= 100;
  temp /= percent;
  *position = (int16_t)temp;
  *position *= MULTIPLYER;
  return STATUS_CODE_OK;
}
