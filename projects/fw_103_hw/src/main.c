#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  uint16_t a6_data = 0;
  AdcChannel *channel = (AdcChannel *)context;
  adc_read_raw(*channel, &a6_data);
  LOG_DEBUG("Pin A6 data: %d\n", a6_data);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  GpioAddress pin_address = { .port = GPIO_PORT_A, .pin = 6 };

  GpioAddress button_address = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&pin_address, &gpio_settings);

  adc_init(ADC_MODE_SINGLE);

  AdcChannel channel = NUM_ADC_CHANNELS;

  adc_get_channel(button_address, &channel);
  adc_set_channel(channel, true);

  gpio_it_register_interrupt(&button_address, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &channel);

  while (true) {
  }

  return 0;
}
