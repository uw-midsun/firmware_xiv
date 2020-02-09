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

#define TIMER_TIMEOUT_IN_MILLIS 100

int16_t brake_position = INT16_MAX;
int16_t throttle_position = INT16_MAX;

// pedal callback
static void prv_pedal_timeout(SoftTimerId timer_id, void *context) {
  PedalDataStorage *pedal_storage = context;

  get_brake_data(pedal_storage, &brake_position);
  get_throttle_data(pedal_storage, &throttle_position);
  // SENDING POSITIONS THROUGH CAN MESSAGES
  CAN_TRANSMIT_PEDAL_OUTPUT((uint32_t)throttle_position, (uint32_t)brake_position);

  soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_pedal_timeout, context, NULL);
}

int16_t get_brake_position() {
  return brake_position;
}

int16_t get_throttle_position() {
  return throttle_position;
}

// main should have a brake fsm, and ads1015storage
StatusCode pedal_data_tx_init(PedalDataStorage *storage) {
  // Throttle Channels, we only use 1 right now
  status_ok_or_return(
      ads1015_configure_channel(storage->storage, storage->throttle_channel1, true, NULL, NULL));
  status_ok_or_return(
      ads1015_configure_channel(storage->storage, storage->throttle_channel2, true, NULL, NULL));
  // brake channel
  status_ok_or_return(
      ads1015_configure_channel(storage->storage, storage->brake_channel, true, NULL, NULL));

  status_ok_or_return(
      soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_pedal_timeout, storage, NULL));
  return STATUS_CODE_OK;
}
