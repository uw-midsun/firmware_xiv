#include "killswitch.h"

#include "bms.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "gpio_it.h"

static void prv_killswitch_handler(const GpioAddress *address, void *context) {
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);
  bool clear = state == GPIO_STATE_LOW;
  if (state == GPIO_STATE_HIGH) {
    fault_bps_set(EE_BPS_STATE_FAULT_KILLSWITCH);
  } else {
    fault_bps_clear(EE_BPS_STATE_FAULT_KILLSWITCH);
  }
}

StatusCode killswitch_init(DebouncerStorage *storage) {
  GpioAddress monitor_pin = KS_MONITOR_PIN;
  GpioAddress enable_pin = KS_ENABLE_PIN;

  GpioSettings enable_pin_settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_HIGH,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };

  // init enable pin
  status_ok_or_return(gpio_init_pin(&enable_pin, &enable_pin_settings));

  // init monitor pin
  status_ok_or_return(debouncer_init_pin(storage, &monitor_pin, prv_killswitch_handler, NULL));

  // Force update
  prv_killswitch_handler(&monitor_pin, NULL);

  return STATUS_CODE_OK;
}
