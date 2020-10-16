#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static GpioAddress s_button_address = { .port = GPIO_PORT_B, .pin = 2 };
static GpioAddress s_adc_address = { .port = GPIO_PORT_A, .pin = 6 };

static GpioSettings s_adc_settings = {
  .direction = GPIO_DIR_IN,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_ANALOG,
};

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  GpioAddress *adc_addr = (GpioAddress *)context;
  uint16_t channel_data = 0;
  adc_read_converted_pin(*adc_addr, &channel_data);
  LOG_DEBUG("%d\n", channel_data);
}

static void prv_register_interrupts(void) {
  gpio_it_register_interrupt(&s_button_address, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &s_adc_address);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();

  gpio_init_pin(&s_adc_address, &s_adc_settings);
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(s_adc_address, true);

  prv_register_interrupts();

  while (true) {
  }

  return 0;
}
