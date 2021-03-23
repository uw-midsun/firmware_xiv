#include <stdint.h>

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static void prv_interrupt_handler(const GpioAddress *address, void *context) {
  AdcChannel *reading_channel = context;
  uint16_t data = 0;
  adc_read_converted(*reading_channel, &data);
  LOG_DEBUG("%d\n", data);
}

int main(void) {
  GpioAddress reading_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings reading_settings = { .direction = GPIO_DIR_IN,
                                    .state = GPIO_STATE_LOW,
                                    .alt_function = GPIO_ALTFN_NONE,
                                    .resistor = GPIO_RES_NONE };

  GpioSettings button_settings = { .direction = GPIO_DIR_IN,
                                   .state = GPIO_STATE_LOW,
                                   .alt_function = GPIO_ALTFN_NONE,
                                   .resistor = GPIO_RES_NONE };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();
  gpio_init_pin(&reading_addr, &reading_settings);
  gpio_init_pin(&button_addr, &button_settings);

  adc_init(ADC_MODE_SINGLE);
  AdcChannel reading_channel = NUM_ADC_CHANNELS;
  adc_get_channel(reading_addr, &reading_channel);
  adc_set_channel(reading_channel, true);
  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_interrupt_handler, &reading_channel);
  while (true) {
  }

  return 0;
}
