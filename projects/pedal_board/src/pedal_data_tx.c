#include "pedal_data_tx.h"
#include "ads1015.h"
#include "brake_data.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"

static uint16_t brake_position = 0;
static uint16_t throttle_position = 0;

uint16_t getBrakePosition() {
  return brake_position;
}

uint16_t getThrottlePosition() {
  return throttle_position;
}

// Timeout callback
static void prv_blink_timeout(SoftTimerId timer_id, void *context) {
  PedalData *data = context;
  Ads1015Channel brake_channel = data->brake_channel;
  Ads1015Channel throttle_channel = data->throttle_channel;

  brake_position = *getBrakeData(data->storage, brake_channel);
  throttle_position = *getBrakeData(data->storage, throttle_channel);
  soft_timer_start_millis(100, prv_blink_timeout, context, NULL);
}

// main should have a brake fsm, and ads1015storage
StatusCode pedal_data_tx_init(Ads1015Storage *storage) {
  ads1015_configure_channel(storage, ADS1015_CHANNEL_0, true, NULL, NULL);
  ads1015_configure_channel(storage, ADS1015_CHANNEL_1, true, NULL, NULL);
  ads1015_configure_channel(storage, ADS1015_CHANNEL_2, true, NULL, NULL);
  PedalData data = {
    .storage = storage, .brake_channel = ADS1015_CHANNEL_2, .throttle_channel = ADS1015_CHANNEL_0
  };

  soft_timer_start_millis(100, prv_blink_timeout, &data, NULL);
}
