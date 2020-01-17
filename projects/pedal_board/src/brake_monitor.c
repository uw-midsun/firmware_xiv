#include "ads1015.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "soft_timer.h"
#include "pedal_events.h"

static void prv_callback_channel1(Ads1015Channel channel, void *context) {
  Ads1015Storage *storage = context;
  int16_t position = 0;
  ads1015_read_raw(storage, channel, &position);

  // math to convert readings to angles
  // position =

  if (position > (3.14 / 4)) {
    event_raise(PEDAL_BRAKE_FSM_EVENT_PRESSED, 1);
  } else {
    event_raise(PEDAL_BRAKE_FSM_EVENT_RELEASED, 0);
  }
}

// Timeout callback
static void prv_blink_timeout(SoftTimerId timer_id, void *context) {
  Ads1015Storage *storage = context;
  prv_callback_channel1(storage->current_channel, storage);
 
  LOG_DEBUG("Reading ADS1015\n");
 
  // Schedule another timer - this creates a periodic timer
  soft_timer_start_seconds(0.05, prv_blink_timeout, storage, NULL);
}

//main should have a brake fsm, and ads1015storage
StatusCode brake_monitor_init(Ads1015Storage *storage) {
  ads1015_configure_channel(storage, storage->current_channel, true, prv_callback_channel1, storage);
  soft_timer_init();
  soft_timer_start_seconds(0.05, prv_blink_timeout, storage, NULL);
  return STATUS_CODE_OK;
}
