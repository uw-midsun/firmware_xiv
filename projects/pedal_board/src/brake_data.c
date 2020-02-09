#include "log.h"
#include "pedal_data_tx.h"
#include "pedal_events.h"
#include "soft_timer.h"

// NEED TO CHANGE!!!!!!
#define UPPER_BRAKE_VALUE 1273
#define LOWER_BRAKE_VALUE 295
#define MULTIPLYER 4096

StatusCode get_brake_data(PedalDataStorage *storage, int16_t *position) {
  status_ok_or_return(ads1015_read_raw(storage->storage, storage->brake_channel, position));
  int32_t percent = UPPER_BRAKE_VALUE - LOWER_BRAKE_VALUE;
  int32_t temp = (int32_t)*position * MULTIPLYER;
  temp -= LOWER_BRAKE_VALUE * MULTIPLYER;
  temp *= 100;
  temp /= percent * MULTIPLYER;
  *position = (int16_t)temp;
  return STATUS_CODE_OK;
}
