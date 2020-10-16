// Common helper functions for BTS7XXX-series load switches
// TODO(SOFT-323) update the functions in bts7xxx_load_switch.c to use these

#include "bts7xxx_common.h"

// put these into the storage or something. could define it in the header as well since they're
// the same, just need to have a way to access them both from this module
#define BTS7XXX_FAULT_RESTART_DELAY_MS 110
#define BTS7XXX_FAULT_RESTART_DELAY_US (BTS7XXX_FAULT_RESTART_DELAY_MS * 1000)

StatusCode bts7xxx_enable_pin(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_HIGH);
  } else {
    return pca9539r_gpio_set_state(pin->enable_pin_pca9539r, PCA9539R_GPIO_STATE_HIGH);
  }
}

// Broad function to disable the pin passed in.
StatusCode bts7xxx_disable_pin(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    return gpio_set_state(pin->enable_pin_stm32, GPIO_STATE_LOW);
  } else {
    return pca9539r_gpio_set_state(pin->enable_pin_pca9539r, PCA9539R_GPIO_STATE_LOW);
  }
}

// Broad function to get whether the pin passed in is enabled.
StatusCode bts7xxx_get_pin_enabled(Bts7xxxEnablePin *pin) {
  if (pin->pin_type == BTS7XXX_PIN_STM32) {
    GpioState pin_state;
    gpio_get_state(pin->enable_pin_stm32, &pin_state);
    return (pin_state == GPIO_STATE_HIGH);
  } else {
    Pca9539rGpioState pin_state;
    pca9539r_gpio_get_state(pin->enable_pin_pca9539r, &pin_state);
    return (pin_state == PCA9539R_GPIO_STATE_HIGH);
  }
}

// Broad pin soft timer cb without re-enabling the pin
void bts7xxx_fault_handler_cb(SoftTimerId timer_id, void *context) {
  Bts7xxxEnablePin *pin = context;
  pin->fault_in_progress = false;
  pin->fault_timer_id = SOFT_TIMER_INVALID_TIMER;
}

// Broad pin re-enable soft timer cb
void bts7xxx_fault_handler_enable_cb(SoftTimerId timer_id, void *context) {
  Bts7xxxEnablePin *pin = context;
  bts7xxx_fault_handler_cb(timer_id, context);
  bts7xxx_enable_pin(pin);
}

// Helper function to clear fault on a given pin
StatusCode bts7xxx_handle_fault_pin(Bts7xxxEnablePin *pin) {
  if (!pin->fault_in_progress) {
    pin->fault_in_progress = true;
    if (bts7xxx_get_pin_enabled(pin)) {
      status_ok_or_return(bts7xxx_disable_pin(pin));
      soft_timer_start(BTS7XXX_FAULT_RESTART_DELAY_US, bts7xxx_fault_handler_enable_cb, pin,
                       &pin->fault_timer_id);
    } else {
      soft_timer_start(BTS7XXX_FAULT_RESTART_DELAY_US, bts7xxx_fault_handler_cb, pin,
                       &pin->fault_timer_id);
    }
  }
  return STATUS_CODE_OK;
}
