#include "debug_led.h"

static const GpioAddress s_leds[NUM_DEBUG_LEDS] = {
  [DEBUG_LED_BLUE_A] = { .port = GPIO_PORT_B, .pin = 5 },
  [DEBUG_LED_BLUE_B] = { .port = GPIO_PORT_B, .pin = 4 },
  [DEBUG_LED_GREEN] = { .port = GPIO_PORT_B, .pin = 3 },
  [DEBUG_LED_RED] = { .port = GPIO_PORT_A, .pin = 15 },
};

StatusCode debug_led_init(DebugLed led) {
  const GpioSettings led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
  };
  return gpio_init_pin(&s_leds[led], &led_settings);
}

StatusCode debug_led_set_state(DebugLed led, bool on) {
  // Active-low LED
  return gpio_set_state(&s_leds[led], (on) ? GPIO_STATE_LOW : GPIO_STATE_HIGH);
}

StatusCode debug_led_toggle_state(DebugLed led) {
  return gpio_toggle_state(&s_leds[led]);
}
