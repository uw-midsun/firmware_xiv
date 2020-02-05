#include "throttle.h"

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "event_queue.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"

static SoftTimerId drive_fsm_soft_timer_id;

static void prv_timer_callback(SoftTimerId timer_id, void *context) {
  Ads1015Storage *storage = context;
  int16_t reading1 = INT16_MIN;
  int16_t reading2 = INT16_MAX;

  if (ads1015_read_raw(storage, ADS1015_CHANNEL_0, &reading1) ==
      STATUS_CODE_INVALID_ARGS) {
    LOG_DEBUG("CHANNEL HAS NOT BEEN CONFIGURED CORRECTLY \n");
  } else {
    LOG_DEBUG("Throttle Main Reading: %d \n", reading1);
  }
  if (ads1015_read_raw(storage, ADS1015_CHANNEL_1, &reading2) ==
      STATUS_CODE_INVALID_ARGS) {
    //LOG_DEBUG("CHANNEL 2 IS WRONG!!!!\n");
  } else {
    LOG_DEBUG("Throttle Secondary Reading: %d \n", reading2);
  }

  uint16_t u_reading =
      (uint16_t)reading1;  // Cast Sreading from int to uint, as ads1015_read_raw assigns a signed
                           // int while event_raise only accepts an unsigned int for the data field

  // TO-DO(SOFT-18): map raw readings to a value that represents throttle position before raising
  event_raise(PEDAL_EVENT_THROTTLE_READING, u_reading);  //

  soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, context, NULL);
}

StatusCode throttle_init(ThrottleStorage *storage, Ads1015Storage *pedal_ads1015_storage) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  status_ok_or_return(
      ads1015_configure_channel(pedal_ads1015_storage, ADS1015_CHANNEL_0, true, NULL, NULL));
  status_ok_or_return(
      ads1015_configure_channel(pedal_ads1015_storage, ADS1015_CHANNEL_1, true, NULL, NULL));
  StatusCode ret =
      soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, pedal_ads1015_storage, NULL);
  return ret;
}

StatusCode throttle_enable(ThrottleStorage *storage) {
  StatusCode ret = soft_timer_start_millis(THROTTLE_UPDATE_PERIOD_MS, prv_timer_callback, storage,
                                           &drive_fsm_soft_timer_id);

  return ret;
}

StatusCode throttle_disable(ThrottleStorage *storage) {
  bool cancel_timer = soft_timer_cancel(drive_fsm_soft_timer_id);

  if (cancel_timer)
    return STATUS_CODE_OK;
  else
    return STATUS_CODE_UNINITIALIZED;
}
