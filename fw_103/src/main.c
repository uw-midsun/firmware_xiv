#include <stdbool.h>
#include <stdint.h>
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "status.h"
#include "wait.h"

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  GpioAddress *adc_address = (GpioAddress *)context;
  uint16_t adc_data = 0;
  adc_read_converted_pin(*(adc_address), &adc_data);
  LOG_DEBUG("ADC Data: %d\n", adc_data);
  delay_ms(200);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  static GpioAddress button_addr = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  static GpioAddress adc_address = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  static GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_HIGH,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };

  GpioSettings adc_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_HIGH,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&button_addr, &button_settings);
  gpio_init_pin(&adc_address, &adc_settings);
  adc_set_channel_pin(adc_address, true);
  adc_init(ADC_MODE_SINGLE);
  // AdcChannel adc_channel = NUM_ADC_CHANNELS;
  // adc_get_channel(adc_address, &adc_channel);
  // adc_set_channel(adc_channel, true);
  // adc_set_channel(ADC_CHANNEL_TEMP, true);

  // LOG_DEBUG("ADC Channel: %d\n", adc_channel);

  gpio_it_register_interrupt(&button_addr, &s_interrupt_settings, INTERRUPT_EDGE_RISING,
                             prv_button_interrupt_handler, NULL);

  while (true) {
    wait();
  }
  return 0;
}
