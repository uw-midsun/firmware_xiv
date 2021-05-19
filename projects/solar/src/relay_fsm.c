#include "relay_fsm.h"

#include <stdbool.h>

#include "can_unpack.h"
#include "data_store.h"
#include "event_queue.h"
#include "fault_handler.h"
#include "fsm.h"
#include "gpio.h"
#include "log.h"
#include "solar_config.h"
#include "solar_events.h"
#include "solar_boards.h"
#include "status.h"

static SolarMpptCount solar_mppt_count;

// look at test_drv120 last test
static void prv_assert_relay(SoftTimerId timer_id, void *context) {
  // Cast context to storage
  RelayFsmStorage *storage = (RelayFsmStorage*)context;
  // Check if prv_realy_err_cb has been called
  if (storage->isErrCalled  == true){
    fault_handler_raise_fault(EE_SOLAR_RELAY_OPEN_ERROR, 0);
  }

  int32_t data_value;
  bool isSet = false;

  data_store_get_is_set(DATA_POINT_CURRENT, &isSet);

  while (!isSet) {
    soft_timer_start_millis(DATA_STORE_ASSERTION_DELAY_MS, prv_assert_relay, context, NULL);
  }

  data_store_get(DATA_POINT_CURRENT, (uint32_t *)&data_value);

  if (data_value > CURRENT_ASSERT_THRESHOLD_uA) {
    fault_handler_raise_fault(EE_SOLAR_FAULT_OVERCURRENT, 0);
  } else {
    // success message => define code_gen ascipd send using can transmit.h
  }
}

static void prv_relay_err_cb(void *context) {
  LOG_DEBUG("RELAY_ERROR CALLBACK\n");
  // Cast context to storage
  RelayFsmStorage *storage = (RelayFsmStorage*)context;
  // Setting error flag true if this function is called
  storage->isErrCalled = true;
  fault_handler_raise_fault(EE_SOLAR_FAULT_DRV120, 0);
}

FSM_DECLARE_STATE(state_relay_open);
FSM_DECLARE_STATE(state_relay_closed);

FSM_STATE_TRANSITION(state_relay_open) {
  FSM_ADD_TRANSITION(SOLAR_RELAY_EVENT_CLOSE, state_relay_closed);
}

FSM_STATE_TRANSITION(state_relay_closed) {
  FSM_ADD_TRANSITION(SOLAR_RELAY_EVENT_OPEN, state_relay_open);
}

static void prv_open_relay(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("Opening relay\n");
  drv120_relay_open();

  // Call the assert function after delay
  soft_timer_start_millis(RELAY_ASSERTION_DELAY_MS, prv_assert_relay, context, NULL);
}

static void prv_close_relay(Fsm *fsm, const Event *e, void *context) {
  LOG_DEBUG("Closing relay\n");
  drv120_relay_close();
}

StatusCode relay_fsm_init(RelayFsmStorage *storage, SolarMpptCount mppt_count) {
  if (storage == NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }
  // Setting storage error flag false intially
  storage->isErrCalled = false;
  // Set the mppt_count
  solar_mppt_count = mppt_count;
  // Init drv120
  Drv120RelaySettings drv120_settings = {
    .enable_pin = config_get_drv120_enable_pin(),
    .status_pin = config_get_drv120_status_pin(),
    .error_handler = prv_relay_err_cb,
    .context = storage,
  };
  status_ok_or_return(drv120_relay_init(&drv120_settings));

  fsm_init(&storage->fsm, "Relay FSM", &state_relay_closed, storage);
  fsm_state_init(state_relay_open, prv_open_relay);
  fsm_state_init(state_relay_closed, prv_close_relay);

  // begin with the relay closed
  drv120_relay_close();

  return STATUS_CODE_OK;
}

bool relay_fsm_process_event(RelayFsmStorage *storage, const Event *e) {
  if (storage == NULL || e == NULL) {
    return false;
  }
  return fsm_process_event(&storage->fsm, e);
}

StatusCode relay_fsm_close(void) {
  return event_raise_priority(RELAY_EVENT_PRIORITY, SOLAR_RELAY_EVENT_CLOSE, 0);
}

StatusCode relay_fsm_open(void) {
  return event_raise_priority(RELAY_EVENT_PRIORITY, SOLAR_RELAY_EVENT_OPEN, 0);
}
