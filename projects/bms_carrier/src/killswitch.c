#include "killswitch.h"

#include "bms.h"
#include "exported_enums.h"
#include "fault_bps.h"
#include "gpio_it.h"
#include "log.h"

static void prv_killswitch_handler(const GpioAddress *address, void *context) {
  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);
  if (state == GPIO_STATE_LOW) {
    LOG_DEBUG("faulting from killswitch\n");
    fault_bps_set(EE_BPS_STATE_FAULT_KILLSWITCH);
  } else {
    LOG_DEBUG("clearing fault from killswitch\n");
    fault_bps_clear(EE_BPS_STATE_FAULT_KILLSWITCH);
  }
}

StatusCode killswitch_init(void) {
  GpioAddress monitor_pin = KS_MONITOR_PIN;
  GpioAddress enable_pin = KS_ENABLE_PIN;

  GpioSettings enable_pin_settings = {
    .direction = GPIO_DIR_OUT,       //
    .state = GPIO_STATE_HIGH,        //
    .resistor = GPIO_RES_NONE,       //
    .alt_function = GPIO_ALTFN_NONE  //
  };

  GpioSettings monitor_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_HIGH,         //
    .alt_function = GPIO_ALTFN_NONE,  //
    .resistor = GPIO_RES_NONE,        //
  };
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_HIGH,
    .type = INTERRUPT_TYPE_INTERRUPT,
  };

  status_ok_or_return(gpio_init_pin(&enable_pin, &enable_pin_settings));
  status_ok_or_return(gpio_init_pin(&monitor_pin, &monitor_settings));

  status_ok_or_return(gpio_it_register_interrupt(
      &monitor_pin, &it_settings, INTERRUPT_EDGE_RISING_FALLING, prv_killswitch_handler, NULL));

  // Force update
  prv_killswitch_handler(&monitor_pin, NULL);

  return STATUS_CODE_OK;
}
