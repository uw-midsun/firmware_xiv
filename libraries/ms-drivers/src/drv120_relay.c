#include "drv120_relay.h"


// Storage for EN pin on DRV120 to open/close relay
static GpioAddress s_relay_pin;

// Initialize pin for output to open/close relay
// Pin taken in is the EN pin on the DRV120: high = closed, low = open
// Relay defaults closed on initialization
StatusCode drv120_relay_init(GpioAddress *pin) {
  s_relay_pin = *pin;

  // Pin high on initialization
  GpioSettings drv120_pin_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  return gpio_init_pin(&s_relay_pin, &drv120_pin_settings);
}

StatusCode drv120_relay_close(void) {
  return gpio_set_state(&s_relay_pin, GPIO_STATE_HIGH);
}

StatusCode drv120_relay_open(void) {
  return gpio_set_state(&s_relay_pin, GPIO_STATE_LOW);
}
