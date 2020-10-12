#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static GpioAddress button_address = { .port = GPIO_PORT_B, .pin = 2 };

static GpioAddress adc_address = { .port = GPIO_PORT_A, .pin = 6 };

static GpioSettings adc_settings = {
  .direction = GPIO_DIR_IN,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_ANALOG,
};

static InterruptSettings interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static AdcChannel adc_pinA6_channel = ADC_CHANNEL_6;

static uint16_t channel_data = 0;

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  adc_read_raw(adc_pinA6_channel, &channel_data);
  LOG_DEBUG("%d\n", channel_data);
}

void prv_register_interrupts(void) {
  gpio_it_register_interrupt(&button_address, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &adc_address);
}

int main(void) {
  adc_init(ADC_MODE_SINGLE);
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  adc_get_channel(adc_address, &adc_pinA6_channel);
  adc_set_channel(adc_pinA6_channel, true);

  prv_register_interrupts();

  while (true) {
  }

  return 0;
}
