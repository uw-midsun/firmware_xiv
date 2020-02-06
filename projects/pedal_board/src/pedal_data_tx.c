#include "pedal_data_tx.h"
#include "ads1015.h"
#include "brake_data.h"
#include "throttle_data.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"

PedalData data = {0};

static int16_t brake_position = 0;
static int16_t throttle_position = 0;

int16_t getBrakePosition() {
  return brake_position;
}

int16_t getThrottlePosition() {
  return throttle_position;
}

// Timeout callback
static void prv_blink_timeout(SoftTimerId timer_id, void *context) {
  PedalData *pedalData = context;

  //getBrakeData(data->storage, data->brake_channel, &brake_position);
  getThrottleData(pedalData->storage, pedalData->throttle_channel, &throttle_position);
  // SENDING POSITIONS THROUGH CAN MESSAGES
  // CAN_TRANSMIT_PEDAL_OUTPUT(brake_position, throttle_position);

  soft_timer_start_millis(100, prv_blink_timeout, context, NULL);
}

// main should have a brake fsm, and ads1015storage
StatusCode pedal_data_tx_init(Ads1015Storage *storage) {
  ads1015_configure_channel(storage, ADS1015_CHANNEL_0, true, NULL, NULL);
  ads1015_configure_channel(storage, ADS1015_CHANNEL_1, true, NULL, NULL);
  ads1015_configure_channel(storage, ADS1015_CHANNEL_2, true, NULL, NULL);
  data.storage = storage;
  data.brake_channel = ADS1015_CHANNEL_2;
  data.throttle_channel = ADS1015_CHANNEL_1;

  StatusCode ret = soft_timer_start_millis(100, prv_blink_timeout, &data, NULL);
  return ret;
}
