#include "killswitch.h"
#include "gpio_it.h"
#include "log.h"

static void prv_killswitch_handler(const GpioAddress *address, void *context) {
  BpsHeartbeatStorage *storage = context;

  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);

  if (state == GPIO_STATE_LOW) {
    // Falling edge - killswitch was hit
    bps_heartbeat_raise_fault(storage, EE_BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  } else {
    // Rising edge - killswitch was released
    bps_heartbeat_clear_fault(storage, EE_BPS_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  }
}

StatusCode killswitch_init(KillswitchStorage *storage, const GpioAddress *killswitch,
                           BpsHeartbeatStorage *bps_heartbeat) {
  // Force update
  prv_killswitch_handler(killswitch, bps_heartbeat);
  return debouncer_init_pin(&storage->debouncer, killswitch, prv_killswitch_handler, bps_heartbeat);
}

StatusCode killswitch_bypass(const GpioAddress *killswitch) {
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };

  // Force high to bypass killswitch
  return gpio_init_pin(killswitch, &gpio_settings);
}
