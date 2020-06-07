#include "ltc_afe_fsm.h"
#include "ltc_afe_impl.h"
#include "plutus_event.h"
#include "soft_timer.h"
#include "log.h"

FSM_DECLARE_STATE(afe_idle);
FSM_DECLARE_STATE(afe_trigger_cell_conv);
FSM_DECLARE_STATE(afe_read_cells);
FSM_DECLARE_STATE(afe_trigger_aux_conv);
FSM_DECLARE_STATE(afe_read_aux);
FSM_DECLARE_STATE(afe_aux_complete);

static bool prv_all_aux_complete(const struct Fsm *fsm, const Event *e, void *context) {
  return e->data >= LTC_AFE_MAX_CELLS_PER_DEVICE;
}

FSM_STATE_TRANSITION(afe_idle) {
  LtcAfeStorage *afe = fsm->context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  FSM_ADD_TRANSITION(afe_events->trigger_cell_conv_event, afe_trigger_cell_conv);
  FSM_ADD_TRANSITION(afe_events->trigger_aux_conv_event, afe_trigger_aux_conv);
}

FSM_STATE_TRANSITION(afe_trigger_cell_conv) {
  LtcAfeStorage *afe = fsm->context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  FSM_ADD_TRANSITION(afe_events->cell_conv_complete_event, afe_read_cells);
  FSM_ADD_TRANSITION(afe_events->fault_event, afe_idle);
}

FSM_STATE_TRANSITION(afe_read_cells) {
  LtcAfeStorage *afe = fsm->context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  FSM_ADD_TRANSITION(afe_events->callback_run_event, afe_idle);
  FSM_ADD_TRANSITION(afe_events->cell_conv_complete_event, afe_read_cells);
  FSM_ADD_TRANSITION(afe_events->fault_event, afe_idle);
}

FSM_STATE_TRANSITION(afe_trigger_aux_conv) {
  LtcAfeStorage *afe = fsm->context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  FSM_ADD_TRANSITION(afe_events->aux_conv_complete_event, afe_read_aux);
  FSM_ADD_TRANSITION(afe_events->fault_event, afe_idle);
}

FSM_STATE_TRANSITION(afe_read_aux) {
  LtcAfeStorage *afe = fsm->context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  FSM_ADD_GUARDED_TRANSITION(afe_events->trigger_aux_conv_event, prv_all_aux_complete,
                             afe_aux_complete);
  FSM_ADD_TRANSITION(afe_events->aux_conv_complete_event, afe_read_aux);

  FSM_ADD_TRANSITION(afe_events->trigger_aux_conv_event, afe_trigger_aux_conv);
  FSM_ADD_TRANSITION(afe_events->fault_event, afe_idle);
}

FSM_STATE_TRANSITION(afe_aux_complete) {
  LtcAfeStorage *afe = fsm->context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  FSM_ADD_TRANSITION(afe_events->callback_run_event, afe_idle);
}

static void prv_cell_conv_timeout(SoftTimerId timer_id, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  event_raise(afe_events->cell_conv_complete_event, 0);
}

static void prv_aux_conv_timeout(SoftTimerId timer_id, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  event_raise(afe_events->aux_conv_complete_event, afe->aux_index);
}

static void prv_afe_trigger_cell_conv_output(struct Fsm *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  StatusCode ret = ltc_afe_impl_trigger_cell_conv(afe);
  if (status_ok(ret)) {
    LOG_DEBUG("TRIGGERED CELL CONVERSION SUCCESSFULLY\n");
    soft_timer_start_millis(LTC_AFE_FSM_CELL_CONV_DELAY_MS, prv_cell_conv_timeout, NULL, NULL);
  } else {
    LOG_DEBUG("FAILED TO TRIGGER CONVERSION\n");
    event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
                         LTC_AFE_FSM_FAULT_TRIGGER_CELL_CONV);
  }
}

static void prv_afe_read_cells_output(struct Fsm *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  StatusCode ret = ltc_afe_impl_read_cells(afe);
  
  LOG_DEBUG("READING FROM CELLS\n");
  if (status_ok(ret)) {
    // Raise the event first in case the user raises a trigger conversion event in the callback
    afe->retry_count = 0;
    event_raise(afe_events->callback_run_event, 0);

    if (afe->settings.cell_result_cb != NULL) {
      LOG_DEBUG("RUNNING CELL CALLBACK\n");
      afe->settings.cell_result_cb(
        afe->cell_voltages,
        afe->settings.num_cells,
        afe->settings.result_context
      );
    }
  } else if (afe->retry_count < LTC_AFE_FSM_MAX_RETRY_COUNT) {
    LOG_DEBUG("TRIGGERING CELL RETRIES\n");
    afe->retry_count++;
    soft_timer_start_millis(LTC_AFE_FSM_CELL_CONV_DELAY_MS, prv_cell_conv_timeout, NULL, NULL);
  } else {
    LOG_DEBUG("FAULT WHILE TRYING TO READ FROM CELLS\n");
    event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
                         LTC_AFE_FSM_FAULT_READ_ALL_CELLS);
  }
}

static void prv_afe_trigger_aux_conv_output(struct Fsm *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  uint32_t device_cell = e->data;
  LOG_DEBUG("TRIGGERING AUX CONVERSION\n");
  StatusCode ret = ltc_afe_impl_trigger_aux_conv(afe, device_cell);
  if (status_ok(ret)) {
    afe->aux_index = device_cell;
    soft_timer_start_millis(LTC_AFE_FSM_AUX_CONV_DELAY_MS, prv_aux_conv_timeout, afe, NULL);
  } else {
    event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
                         LTC_AFE_FSM_FAULT_TRIGGER_AUX_CONV);
  }
}

static void prv_afe_read_aux_output(struct Fsm *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;

  uint16_t device_cell = e->data;

  LOG_DEBUG("READING AUX CONVERSION OUTPUT\n");
  StatusCode ret = ltc_afe_impl_read_aux(afe, device_cell);
  if (status_ok(ret)) {
    // Kick-off the next aux conversion
    afe->retry_count = 0;
    event_raise(afe_events->trigger_aux_conv_event, device_cell + 1);
  } else if (afe->retry_count < LTC_AFE_FSM_MAX_RETRY_COUNT) {
    // Attempt to retry the read after delaying
    afe->retry_count++;
    soft_timer_start_millis(LTC_AFE_FSM_AUX_CONV_DELAY_MS, prv_aux_conv_timeout, afe, NULL);
  } else {
    event_raise_priority(EVENT_PRIORITY_HIGHEST, afe_events->fault_event,
                         LTC_AFE_FSM_FAULT_READ_AUX);
  }
}

static void prv_afe_aux_complete_output(struct Fsm *fsm, const Event *e, void *context) {
  LtcAfeStorage *afe = context;
  LtcAfeEventList *afe_events = &afe->settings.ltc_events;
  LOG_DEBUG("RUNNING CALLBACK FOR AUX OUTPUT\n");

  // Raise the event first in case the user raises a trigger conversion event in the callback
  event_raise(afe_events->callback_run_event, 0);

  // 12 aux conversions complete - the array should be fully populated
  if (afe->settings.aux_result_cb != NULL) {
    afe->settings.aux_result_cb(afe->aux_voltages, afe->settings.num_cells, afe->settings.result_context);
  }
}

StatusCode ltc_afe_fsm_init(Fsm *fsm, LtcAfeStorage *afe) {
  fsm_state_init(afe_idle, NULL);
  fsm_state_init(afe_trigger_cell_conv, prv_afe_trigger_cell_conv_output);
  fsm_state_init(afe_read_cells, prv_afe_read_cells_output);
  fsm_state_init(afe_trigger_aux_conv, prv_afe_trigger_aux_conv_output);
  fsm_state_init(afe_read_aux, prv_afe_read_aux_output);
  fsm_state_init(afe_aux_complete, prv_afe_aux_complete_output);

  fsm_init(fsm, "LTC AFE FSM", &afe_idle, afe);

  return STATUS_CODE_OK;
}
