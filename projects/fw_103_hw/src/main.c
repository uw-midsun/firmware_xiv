#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  uint16_t A6_data = 0;
  AdcChannel *channel = (AdcChannel *)context adc_read_raw(channel, &A6_data);
  LOG_DEBUG("Pin A6 data: %d\n", A6_data);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  GpioAdress pin_address = { .port = GPIO_PORT_A, .pin = 6 };

  GpioAdress button_address = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&pin_address, &settings);

  adc_init(ADC_MODE_SINGLE);

  AdcChannel channel = NUM_ADC_CHANNELS;

  adc_get_channel(&button_address, &channel);
  adc_set_channel(channel, true);

  gpio_it_register_interrupt(&button_address, &s_interrupt_settings, INTERRUPT_EDGE_RISING,
                             prv_button_interrupt_handler, &channel);

  while (true) {
  }

  return 0;
}
