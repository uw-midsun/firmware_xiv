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
#include "pedal_shared_resources_provider.h"
#include "soft_timer.h"
#include "throttle_data.h"

#define TIMER_TIMEOUT_IN_MILLIS 100

int16_t brake_position = INT16_MAX;
int16_t throttle_position = INT16_MAX;

// pedal callback
static void prv_pedal_timeout(SoftTimerId timer_id, void *context) {
  get_brake_data(&brake_position);
  get_throttle_data(&throttle_position);
  // SENDING POSITIONS THROUGH CAN MESSAGES
  LOG_DEBUG("throttle: %d, brake; %d\n", throttle_position, brake_position);
  CAN_TRANSMIT_PEDAL_OUTPUT((uint32_t)throttle_position, (uint32_t)brake_position);
  soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_pedal_timeout, context, NULL);
}

// main should have a brake fsm, and ads1015storage
StatusCode pedal_data_tx_init() {
  status_ok_or_return(
      soft_timer_start_millis(TIMER_TIMEOUT_IN_MILLIS, prv_pedal_timeout, NULL, NULL));
  return STATUS_CODE_OK;
}
