#include "killswitch.h"
#include "gpio_it.h"
#include "log.h"
// #include "debouncer.h"

static void prv_killswitch_handler(const GpioAddress *address, void *context) {
  BatteryHeartbeatStorage *storage = context;

  GpioState state = NUM_GPIO_STATES;
  gpio_get_state(address, &state);

  if (state == GPIO_STATE_LOW) {
    // Falling edge - killswitch was hit
    battery_heartbeat_raise_fault(storage, EE_BATTERY_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  } else {
    // Rising edge - killswitch was released
    battery_heartbeat_clear_fault(storage, EE_BATTERY_HEARTBEAT_FAULT_SOURCE_KILLSWITCH);
  }
}

StatusCode killswitch_init(KillswitchStorage *storage, const GpioAddress *killswitch,
                           BatteryHeartbeatStorage *battery_heartbeat) {
  // Force update
  prv_killswitch_handler(killswitch, battery_heartbeat);
  return debouncer_init_pin(&storage->debouncer, killswitch, prv_killswitch_handler, battery_heartbeat);
}

StatusCode killswitch_bypass(const GpioAddress *killswitch) {
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };

  // Force high to bypass killswitch
  return gpio_init_pin(killswitch, &gpio_settings);
}
