#include "drv120_relay.h"

// Initialize pin for output
StatusCode drv120_relay_init(GpioAddress *pin) {
  relay_pin = *pin;

  // Closed by default
  GpioSettings drv120_pin_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  return gpio_init_pin(&relay_pin, &drv120_pin_settings);
}

StatusCode drv120_relay_close(void) {
  return gpio_set_state(&relay_pin, GPIO_STATE_HIGH);
}

StatusCode drv120_relay_open(void) {
  return gpio_set_state(&relay_pin, GPIO_STATE_LOW);
}
