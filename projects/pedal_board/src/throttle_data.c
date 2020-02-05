#include "brake_data.h"
#include "ads1015.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"

// Timeout callback
uint16_t *getThrottleData(Ads1015Storage *storage, Ads1015Channel *channel) {
  uint16_t position = INT16_MAX;
  ads1015_read_raw(storage, *channel, &position);

  LOG_DEBUG("Brake: %d", position);

  // math to convert readings to angles
  // position =
  return &position;
  // if (position > (3.14 / 4)) {
  //   event_raise(PEDAL_BRAKE_FSM_EVENT_PRESSED, 1);
  // } else {
  //   event_raise(PEDAL_BRAKE_FSM_EVENT_RELEASED, 0);
  // }
}