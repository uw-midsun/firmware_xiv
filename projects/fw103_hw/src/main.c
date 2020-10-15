#include <stdint.h>  // for integer types

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

static GpioAddress s_adc_addr = { .port = GPIO_PORT_A, .pin = 6 };
static GpioAddress s_button_addr = { .port = GPIO_PORT_B, .pin = 2 };

static GpioSettings s_adc_settings = {
  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_ANALOG,  // a connection to peripherals
};

// static GpioSettings s_button_settings = {
//   .direction = GPIO_DIR_IN,
//   .resistor = GPIO_RES_PULLDOWN,
//   .alt_function = GPIO_ALTFN_NONE, // no peripherals
// };

static InterruptSettings s_interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                                  .priority = INTERRUPT_PRIORITY_NORMAL };

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  GpioAddress *adc_channel = (GpioAddress *)context;
  uint16_t adc_data = 0;
  adc_read_converted_pin(*adc_channel, &adc_data);
  LOG_DEBUG("%d\n", adc_data);
}

static void prv_register_interrupts(void) {
  gpio_it_register_interrupt(&s_button_addr, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &s_adc_addr);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_init_pin(&s_adc_addr, &s_adc_settings);
  gpio_it_init();  // requires GPIO and interrupts to be initialized first

  adc_init(ADC_MODE_SINGLE);

  AdcChannel adc_channel = NUM_ADC_CHANNELS;
  adc_get_channel(s_adc_addr, &adc_channel);
  adc_set_channel(adc_channel, true);

  prv_register_interrupts();

  while (true) {
  }

  return 0;
}
