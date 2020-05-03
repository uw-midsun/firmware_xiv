#include "ltc_afe.h"
#include "critical_section.h"
#include "ltc_afe_fsm.h"
#include "ltc_afe_impl.h"
#include "plutus_event.h"

StatusCode ltc_afe_init(LtcAfeStorage *afe, const LtcAfeSettings *settings) {
  status_ok_or_return(ltc_afe_impl_init(afe, settings));
  return ltc_afe_fsm_init(&afe->fsm, afe);
}

StatusCode ltc_afe_set_result_cbs(LtcAfeStorage *afe, LtcAfeResultCallback cell_result_cb,
                                  LtcAfeResultCallback aux_result_cb, void *context) {
  bool disabled = critical_section_start();
  afe->cell_result_cb = cell_result_cb;
  afe->aux_result_cb = aux_result_cb;
  afe->result_context = context;
  critical_section_end(disabled);

  return STATUS_CODE_OK;
}

StatusCode ltc_afe_request_cell_conversion(LtcAfeStorage *afe) {
  return event_raise(PLUTUS_EVENT_AFE_TRIGGER_CELL_CONV, 0);
}

StatusCode ltc_afe_request_aux_conversion(LtcAfeStorage *afe) {
  return event_raise(PLUTUS_EVENT_AFE_TRIGGER_AUX_CONV, 0);
}

bool ltc_afe_process_event(LtcAfeStorage *afe, const Event *e) {
  return fsm_process_event(&afe->fsm, e);
}

StatusCode ltc_afe_toggle_cell_discharge(LtcAfeStorage *afe, uint16_t cell, bool discharge) {
  return ltc_afe_impl_toggle_cell_discharge(afe, cell, discharge);
}
