#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress ADC_addr = { .port = GPIO_PORT_A, .pin = 6 };

  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings ADC_reading_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_HIGH,
  };

  gpio_init_pin(&ADC_addr, &ADC_reading_settings);
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(ADC_addr, true);

  gpio_init_pin(&button_addr, &button_settings);

  static void prv_interrupt_callback(const GpioAddress *address, void *context) {
    uint16_t ADC_reading = 0;
    adc_read_raw_pin(ADC_addr, &ADC_reading);
    LOG_DEBUG(%d\n, ADC_reading);
  }

  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_interrupt_callback, NULL);

  return 0;
}
