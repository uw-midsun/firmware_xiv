#include <stdint.h>

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

static void prv_button_callback(const GpioAddress *address, void *context) {
  uint16_t reading = 0;
  GpioAddress *reading_pin = context;
  adc_read_converted_pin(*reading_pin, &reading);
  LOG_DEBUG("reading in pin A6: %d\n", reading);
}

int main(void) {
  // Library Initializations
  interrupt_init();
  gpio_init();
  gpio_it_init();

  // Pin initializations
  GpioAddress reading_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_pin_addr = { .port = GPIO_PORT_B, .pin = 2 };

  // Settings structs initializations
  GpioSettings reading_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,  // Button is active high (off initially)
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,  // This pin doesn't need to do anything special
  };

  InterruptSettings button_rising = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&reading_addr, &reading_settings);
  gpio_init_pin(&button_pin_addr, &button_pin_settings);

  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(reading_addr, true);

  gpio_it_register_interrupt(&button_pin_addr, &button_rising, INTERRUPT_EDGE_FALLING,
                             prv_button_callback, &reading_addr);

  while (1) {
    wait();
  }

  return 0;
}
