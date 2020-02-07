#include "brake_data.h"
#include "ads1015.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "soft_timer.h"

StatusCode getBrakeData(Ads1015Storage *storage, Ads1015Channel channel, int16_t *position) {
  status_ok_or_return(ads1015_read_raw(storage, channel, position));
  // need to figure out the actual values
  float percent = 1273 - 297;
  float temp = *position;
  temp -= 295;
  temp *= 100 / percent;
  *position = (int16_t)temp;
  *position *= 4096;
  return STATUS_CODE_OK;
}
