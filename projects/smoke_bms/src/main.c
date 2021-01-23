#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// ks toggle = toggle relay state + log / check if set correctly
// periodic AFE temp and voltage readings
// periodic current readings
// passive balancing

static void prv_ks_handler(const GpioAddress *addr, void *context) {
  uint8_t state = 2;
  gpio_get_state(addr, &state);
  LOG_DEBUG("pin %d.%d state %d\n", addr->port, addr->pin, state);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  GpioAddress debug_led = { .port = GPIO_PORT_B, .pin = 3 };
  GpioAddress en = { .port = GPIO_PORT_B, .pin = 9 };
  GpioAddress ks = { .port = GPIO_PORT_A, .pin = 15 };
  GpioAddress r_gnd = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress r_hv = { .port = GPIO_PORT_A, .pin = 0 };
  GpioSettings debug_led_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };
  GpioSettings en_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_HIGH,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };
  GpioSettings ks_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };
  gpio_init_pin(&debug_led, &debug_led_settings);
  gpio_init_pin(&en, &en_settings);
  gpio_init_pin(&ks, &ks_settings);
  InterruptSettings it_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,
    .type = INTERRUPT_TYPE_INTERRUPT,
  };
  gpio_it_register_interrupt(&ks, &it_settings, INTERRUPT_EDGE_RISING_FALLING, prv_ks_handler,
                             NULL);
  gpio_init_pin(&r_gnd, &en_settings);
  gpio_init_pin(&r_hv, &en_settings);
  while (true) {
    gpio_toggle_state(&debug_led);
    delay_ms(500);
  }
}
