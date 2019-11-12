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

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  GpioAddress *led_address = (GpioAddress *)context;
  gpio_toggle_state(led_address);
}

static GpioSettings s_led_settings = { .direction = GPIO_DIR_OUT,
                                       .state = GPIO_STATE_HIGH,
                                       .alt_function = GPIO_ALTFN_NONE,
                                       .resistor = GPIO_RES_NONE };

void prv_init_leds(void) {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    gpio_init_pin(&led_addresses[i], &s_led_settings);
  }
}

void prv_register_interrupts(void) {
  gpio_it_register_interrupt(&button_addresses[BUTTON_GREEN], &s_interrupt_settings,
                             INTERRUPT_EDGE_RISING, prv_button_interrupt_handler,
                             &led_addresses[LED_BLUE]);
  gpio_it_register_interrupt(&button_addresses[BUTTON_YELLOW], &s_interrupt_settings,
                             INTERRUPT_EDGE_FALLING, prv_button_interrupt_handler,
                             &led_addresses[LED_YELLOW]);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  prv_init_leds();
  prv_register_interrupts();

  while (true) {
  }

  return 0;
}
