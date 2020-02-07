#include "brake_data.h"
#include "ads1015.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "soft_timer.h"

// Timeout callback
StatusCode getBrakeData(Ads1015Storage *storage, Ads1015Channel channel, int16_t *position) {
  status_ok_or_return(ads1015_read_raw(storage, channel, position));
  // need to figure out the actual values
  int16_t percent = 1688;
  *position -= 148;
  *position /= percent;
  *position *= 4096;
  LOG_DEBUG("Brake: %d \n", *position);
  // math to convert readings to angles
  // position =
  // if (position > (3.14 / 4)) {
  //   event_raise(PEDAL_BRAKE_FSM_EVENT_PRESSED, 1);
  // } else {
  //   event_raise(PEDAL_BRAKE_FSM_EVENT_RELEASED, 0);
  // }
  return STATUS_CODE_OK;
}
