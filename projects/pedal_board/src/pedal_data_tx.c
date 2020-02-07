#include "pedal_data_tx.h"
#include "ads1015.h"
#include "brake_data.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "can_unpack.h"
#include "event_queue.h"
#include "fsm.h"
#include "log.h"
#include "pedal_events.h"
#include "soft_timer.h"
#include "throttle_data.h"

PedalData data = { 0 };

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

  // getBrakeData(data->storage, data->brake_channel, &brake_position);
  getThrottleData(pedalData->storage, pedalData->throttle_channel, &throttle_position);
  // SENDING POSITIONS THROUGH CAN MESSAGES
  LOG_DEBUG("Throttle: %d \n", throttle_position);
  // LOG_DEBUG("Brake: %d \n", brake_position);
  CAN_TRANSMIT_PEDAL_OUTPUT((uint32_t)throttle_position, (uint32_t)brake_position);

  soft_timer_start_millis(100, prv_blink_timeout, context, NULL);
}

// main should have a brake fsm, and ads1015storage
StatusCode pedal_data_tx_init(Ads1015Storage *storage, CanStorage *can_storage,
                              CanSettings *can_settings) {
  status_ok_or_return(can_init(can_storage, can_settings));
  // don't need to register a rx handler

  // Throttle Channels, we only use 1 right now
  status_ok_or_return(ads1015_configure_channel(storage, ADS1015_CHANNEL_0, true, NULL, NULL));
  status_ok_or_return(ads1015_configure_channel(storage, ADS1015_CHANNEL_1, true, NULL, NULL));
  // brake channel
  status_ok_or_return(ads1015_configure_channel(storage, ADS1015_CHANNEL_2, true, NULL, NULL));
  data.storage = storage;
  data.brake_channel = ADS1015_CHANNEL_2;
  data.throttle_channel = ADS1015_CHANNEL_1;

  StatusCode ret = soft_timer_start_millis(100, prv_blink_timeout, &data, NULL);
  return ret;
}
