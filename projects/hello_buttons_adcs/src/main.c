#include <stdbool.h>
#include <stdint.h>

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static GpioAddress s_adc_pin_addr = { .port = GPIO_PORT_A, .pin = 6 };
static GpioAddress s_button_addr = { .port = GPIO_PORT_B, .pin = 2 };

static GpioSettings s_adc_pin_settings = {
  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_ANALOG,
};

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

static AdcChannel s_adc_pin_channel = NUM_ADC_CHANNELS;

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  AdcChannel *s_adc_pin_channel = context;
  
  uint16_t adc_pin_data = 0;

  adc_read_converted(*s_adc_pin_channel, &adc_pin_data);
  LOG_DEBUG("ADC data: %d\n", adc_pin_data);
}

static void prv_register_interrupts(void) {
  gpio_it_register_interrupt(&s_button_addr, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &s_adc_pin_channel);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  gpio_it_init();
  prv_register_interrupts();

  gpio_init_pin(&s_adc_pin_addr, &s_adc_pin_settings);

  adc_init(ADC_MODE_SINGLE);
  adc_get_channel(s_adc_pin_addr, &s_adc_pin_channel);
  adc_set_channel(s_adc_pin_channel, true);

  while (true) {
  }

  return 0;
}
