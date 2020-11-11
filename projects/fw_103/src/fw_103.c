#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// interrupt callback function
void prv_button_pressed(const GpioAddress *address, void *context) {
  GpioAddress *adc_read = (GpioAddress *)context;
  uint16_t adc_value = 0;
  adc_read_converted_pin(*adc_read, &adc_value);
  LOG_DEBUG("%d\n", adc_value);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress adc_reading_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings adc_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&adc_reading_addr, &adc_settings);
  gpio_init_pin(&button_addr, &button_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel adc_channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_reading_addr, &adc_channel);
  adc_set_channel(adc_reading_addr, true);

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_pressed, &adc_reading_addr);

  while (true) {
    wait();
  }
  return 0;
}
