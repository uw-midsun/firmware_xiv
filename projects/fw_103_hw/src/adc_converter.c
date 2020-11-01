#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_button_press(const GpioAddress *address, void *context) {
  uint16_t adc_data = 0;
  GpioAddress *read_channel = (GpioAddress *)context;
  adc_read_converted(*read_channel, &adc_data);
  LOG_DEBUG("%d\n", adc_data);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress read_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings read_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor GPIO_RES_NONE,
    .alt_function GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor GPIO_RES_NONE,
    .alt_function GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&read_addr, &read_settings);
  gpio_init_pin(&button_addr, &button_settings);

  adc_init(ADC_MODE_SINGLE);

  AdcChannel read_channel = NUM_ADC_CHANNELS;
  adc_get_channel(read_addr, &read_channel);
  adc_set_channel(read_channel, true);

  InterruptSettings settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&button_addr, &settings, INTERRUPT_EDGE_FALLING, prv_button_press,
                             &read_addr);

  while (true) {
    wait();
  }

  return 0;
}
