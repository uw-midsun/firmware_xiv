#include "ltc_afe.h"
#include "critical_section.h"
#include "ltc_afe_fsm.h"
#include "ltc_afe_impl.h"

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  status_ok_or_return(ltc_afe_impl_init(afe, settings));
  return ltc_afe_fsm_init(&afe->fsm, afe);
}

StatusCode ltc_afe_set_result_cbs(LtcAfeStorage *afe, LtcAfeResultCallback cell_result_cb,
                                  LtcAfeResultCallback aux_result_cb, void *context) {
  LtcAfeSettings *settings = &afe->settings;
  if (settings->cell_result_cb != NULL || settings->aux_result_cb != NULL) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  bool disabled = critical_section_start();
  settings->cell_result_cb = cell_result_cb;
  settings->aux_result_cb = aux_result_cb;
  settings->result_context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_request_cell_conversion(LtcAfeStorage *afe) {
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;
  return event_raise(afe_events->trigger_cell_conv_event, 0);
}

StatusCode ltc_afe_request_aux_conversion(LtcAfeStorage *afe) {
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;
  return event_raise(afe_events->trigger_aux_conv_event, 0);
}

bool ltc_afe_process_event(LtcAfeStorage *afe, const Event *e) {
  return fsm_process_event(&afe->fsm, e);
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return ltc_afe_impl_toggle_cell_discharge(afe, cell, discharge);
}
