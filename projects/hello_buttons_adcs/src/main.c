#include <stdint.h>

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "interrupt.h"
#include "soft_timer.h"

static GpioAddress adc_pin_addr = { .port = GPIO_PORT_A, .pin = 6 };
static GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

static GpioSettings adc_pin_settings = {
  GPIO_DIR_IN,
  GPIO_STATE_LOW,
  GPIO_RES_NONE,
  GPIO_ALTFN_ANALOG,
};

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

static AdcChannel adc_pin_channel = NUM_ADC_CHANNELS;

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  AdcChannel *adc_pin_channel = context;
  uint16_t adc_pin_data = 0;

  adc_read_converted(*adc_pin_channel, &adc_pin_data);
  LOG_DEBUG("ADC data: %d\n", adc_pin_data);
}

void prv_register_interrupts(void) {
  adc_get_channel(adc_pin_addr, &adc_pin_channel);
  adc_set_channel(adc_pin_channel, true);

  gpio_it_register_interrupt(&button_addr, &s_interrupt_settings, INTERRUPT_EDGE_RISING,
                             prv_button_interrupt_handler, &adc_pin_channel);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  gpio_it_init();
  prv_register_interrupts();

  gpio_init_pin(&adc_pin_addr, &adc_pin_settings);

  adc_init(ADC_MODE_SINGLE);

  while (true) {
  }

  return 0;
}
