#include "solar_fsm.h"

#include <stdint.h>

#include "drv120_relay.h"
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
