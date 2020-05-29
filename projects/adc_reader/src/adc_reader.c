#include "adc.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"

int main(void) {
  interrupt_init();
  gpio_init();

  GpioAddress reader_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings reader_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };

  gpio_init_pin(&reader_addr, &reader_settings);
  gpio_init_pin(&button_addr, &button_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel reader_channel = NUM_ADC_CHANNELS;

  adc_get_channel(reader_addr, &reader_channel);
  adc_set_channel(reader_channel, true);

  while (true) {
  }
}
