#include "pedal_monitor.h"
#include "centre_console_events.h"

static PedalMonitorStorage s_storage = { 0 };

static void prv_update_state(SoftTimerId timer_id, void *context) {
  PedalMonitorStorage *storage = context;
  PedalState current_state = storage->state;
  PedalValues pedal_values = pedal_rx_get_pedal_values(&storage->rx_storage);
  storage->state =
      (pedal_values.brake > PEDAL_STATE_THRESHOLD) ? PEDAL_STATE_PRESSED : PEDAL_STATE_RELEASED;
  brake_light_control_update(current_state, storage->state);
  soft_timer_start_millis(PEDAL_STATE_UPDATE_FREQUENCY_MS, prv_update_state, &s_storage,
                          &s_storage.timer_id);
}

StatusCode pedal_monitor_init(void) {
  PedalRxSettings settings = { .timeout_event = PEDAL_MONITOR_RX_TIMED_OUT,
                               .timeout_ms = PEDAL_RX_TIMEOUT_MS };
  status_ok_or_return(pedal_rx_init(&s_storage.rx_storage, &settings));
  status_ok_or_return(soft_timer_start_millis(PEDAL_STATE_UPDATE_FREQUENCY_MS, prv_update_state,
                                              &s_storage, &s_storage.timer_id));
  return STATUS_CODE_OK;
}

PedalState get_pedal_state(void) {
  return s_storage.state;
}
