#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_interrupt_callback_fn(const GpioAddress *address, void *context) {
  GpioAddress *adc_address = (GpioAddress *)context;
  uint16_t value = 0;
  adc_read_converted_pin(*adc_address, &value);
  LOG_DEBUG("Converted ADC value is %d\n", value);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  soft_timer_init();
  GpioAddress adc_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };
  GpioSettings adc_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_ANALOG,
    .resistor = GPIO_RES_NONE,
  };
  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };
  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  gpio_init_pin(&adc_addr, &adc_settings);
  gpio_init_pin(&button_addr, &button_settings);
  adc_init(ADC_MODE_SINGLE);
  AdcChannel adc_channel = NUM_ADC_CHANNELS;
  adc_get_channel(adc_addr, &adc_channel);
  adc_set_channel(adc_channel, true);
  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_interrupt_callback_fn, &adc_addr);
  while (true) {
    wait();
  }
  return 0;
}
