#include "bts7xxx_common_impl.h"

void bts7xxx_fault_handler_cb(SoftTimerId timer_id, void *context) {
  Bts7xxxEnablePin *pin = context;
  pin->fault_in_progress = false;
  pin->fault_timer_id = SOFT_TIMER_INVALID_TIMER;
}

void bts7xxx_fault_handler_enable_cb(SoftTimerId timer_id, void *context) {
  Bts7xxxEnablePin *pin = context;
  bts7xxx_fault_handler_cb(timer_id, context);
  bts7xxx_enable_pin(pin);
}

StatusCode bts7xxx_handle_fault_pin(Bts7xxxEnablePin *pin) {
  if (!pin->fault_in_progress) {
    if (bts7xxx_get_pin_enabled(pin)) {
      status_ok_or_return(bts7xxx_disable_pin(pin));
      status_ok_or_return(soft_timer_start(BTS7XXX_FAULT_RESTART_DELAY_US,
                                           bts7xxx_fault_handler_enable_cb, pin,
                                           &pin->fault_timer_id));
    } else {
      status_ok_or_return(soft_timer_start(BTS7XXX_FAULT_RESTART_DELAY_US, bts7xxx_fault_handler_cb,
                                           pin, &pin->fault_timer_id));
    }
    // Only set fault_in_progress to true after we're sure that setting up
    // fault handling procedures has happened successfully
    pin->fault_in_progress = true;
  }
  return STATUS_CODE_OK;
}
