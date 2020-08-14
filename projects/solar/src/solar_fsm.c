#include "solar_fsm.h"

#include <stdint.h>

#include "can.h"
#include "can_msg_defs.h"
#include "can_unpack.h"
#include "drv120_relay.h"
#include "exported_enums.h"
#include "fsm.h"
#include "solar_events.h"
#include "status.h"

FSM_DECLARE_STATE(state_relay_open);
FSM_DECLARE_STATE(state_relay_closed);

FSM_STATE_TRANSITION(state_relay_open) {
  FSM_ADD_TRANSITION(SOLAR_COMMAND_EVENT_CLOSE_RELAY, state_relay_closed);
}

FSM_STATE_TRANSITION(state_relay_closed) {
  SolarFsmStorage *storage = fsm->context;
  for (uint8_t i = 0; i < storage->num_relay_open_events; i++) {
    FSM_ADD_TRANSITION(storage->relay_open_events[i], state_relay_open);
  }
  FSM_ADD_TRANSITION(SOLAR_COMMAND_EVENT_OPEN_RELAY, state_relay_open);
}

static void prv_open_relay(Fsm *fsm, const Event *e, void *context) {
  drv120_relay_open();
}

static void prv_close_relay(Fsm *fsm, const Event *e, void *context) {
  drv120_relay_close();
}

static StatusCode prv_handle_relay_state_can(const CanMessage *msg, void *context,
                                             CanAckStatus *ack) {
  // mirror the battery relay state
  uint16_t relay_mask, relay_state;
  CAN_UNPACK_SET_RELAY_STATES(msg, &relay_mask, &relay_state);

  if (relay_mask & (1 << EE_RELAY_ID_BATTERY)) {
    if (relay_state & (EE_RELAY_STATE_CLOSE << EE_RELAY_ID_BATTERY)) {
      return event_raise_no_data(SOLAR_COMMAND_EVENT_CLOSE_RELAY);
    } else {
      return event_raise_no_data(SOLAR_COMMAND_EVENT_OPEN_RELAY);
    }
  }

  return STATUS_CODE_OK;
}

StatusCode solar_fsm_init(SolarFsmStorage *storage, const SolarFsmSettings *settings) {
  if (storage == NULL || settings == NULL ||
      settings->num_relay_open_events > MAX_RELAY_OPEN_EVENTS) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  for (uint8_t i = 0; i < settings->num_relay_open_events; i++) {
    storage->relay_open_events[i] = settings->relay_open_events[i];
  }
  storage->num_relay_open_events = settings->num_relay_open_events;

  fsm_init(&storage->fsm, "Solar FSM", &state_relay_closed, storage);
  fsm_state_init(state_relay_open, prv_open_relay);
  fsm_state_init(state_relay_closed, prv_close_relay);

  can_register_rx_handler(SYSTEM_CAN_MESSAGE_SET_RELAY_STATES, prv_handle_relay_state_can, NULL);

  // begin with the relay closed
  drv120_relay_close();

  return STATUS_CODE_OK;
}

bool solar_fsm_process_event(SolarFsmStorage *storage, const Event *e) {
  if (storage == NULL || e == NULL) {
    return false;
  }
  return fsm_process_event(&storage->fsm, e);
}
