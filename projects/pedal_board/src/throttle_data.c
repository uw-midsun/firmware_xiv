#include "ads1015.h"
#include "brake_data.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"

// Timeout callback
void getThrottleData(Ads1015Storage *storage, Ads1015Channel channel, int16_t *position) {
  if (ads1015_read_raw(storage, channel, position) == STATUS_CODE_INVALID_ARGS) {
    LOG_DEBUG("CHANNEL HAS NOT BEEN CONFIGURED CORRECTLY \n");
  }
  float percent = 1273 - 297;
  float temp = *position;
  temp -= 295;
  temp *= 100 / percent;
  *position = (int16_t)temp;
  LOG_DEBUG("Throttle: %d \n", *position);
  //*position *= 4096;

  // math to convert readings to angles
  // position =
  // if (position > (3.14 / 4)) {
  //   event_raise(PEDAL_BRAKE_FSM_EVENT_PRESSED, 1);
  // } else {
  //   event_raise(PEDAL_BRAKE_FSM_EVENT_RELEASED, 0);
  // }
}