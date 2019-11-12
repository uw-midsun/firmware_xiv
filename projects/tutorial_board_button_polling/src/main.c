#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

typedef enum { BUTTON_GREEN = 0, BUTTON_YELLOW, NUM_BUTTONS } Button;

typedef enum { LED_BLUE = 0, LED_YELLOW, NUM_LEDS } Led;

static GpioAddress led_addresses[] = {
  [LED_BLUE] = { .port = GPIO_PORT_B, .pin = 3 },
  [LED_YELLOW] = { .port = GPIO_PORT_B, .pin = 4 },
};

static GpioAddress button_addresses[] = {
  [BUTTON_GREEN] = { .port = GPIO_PORT_A, .pin = 7 },
  [BUTTON_YELLOW] = { .port = GPIO_PORT_A, .pin = 6 },
};

static GpioSettings s_button_settings = {
  .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_NONE, .resistor = GPIO_RES_PULLDOWN
};

static GpioSettings s_led_settings = { .direction = GPIO_DIR_OUT,
                                       .state = GPIO_STATE_HIGH,
                                       .alt_function = GPIO_ALTFN_NONE,
                                       .resistor = GPIO_RES_NONE };

void prv_init_gpio_pins(void) {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    gpio_init_pin(&led_addresses[i], &s_led_settings);
    gpio_init_pin(&button_addresses[i], &s_button_settings);
  }
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  prv_init_gpio_pins();

  while (true) {
    GpioState state_green = NUM_GPIO_STATES;
    gpio_get_state(&button_addresses[BUTTON_GREEN], &state_green);
    gpio_set_state(&led_addresses[LED_BLUE], state_green);

    GpioState state_yellow = NUM_GPIO_STATES;
    gpio_get_state(&button_addresses[BUTTON_YELLOW], &state_yellow);
    gpio_set_state(&led_addresses[LED_YELLOW], state_yellow);
  }

  return 0;
}
