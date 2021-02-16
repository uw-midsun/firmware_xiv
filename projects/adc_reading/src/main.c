#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "soft_timer.h"

static void prv_interrupt_callback(const GpioAddress *address, void *context) {
  // casting void* back for use outside main()
  AdcChannel *adc_reading_channel = context;

  // read and print adc reading data
  uint16_t adc_reading_data = 0;
  adc_read_converted(adc_reading_channel, &adc_reading_data);
  LOG_DEBUG("ADC Reading Data: %d\n", adc_reading_data);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress adc_reading_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings adc_reading_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_PULLDOWN,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  InterruptSettings button_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&adc_reading_addr, &adc_reading_settings);
  gpio_init_pin(&button_addr, &button_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel adc_reading_channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_reading_addr, &adc_reading_channel);
  adc_set_channel(adc_reading_channel, true);

  LOG_DEBUG("ADC Reading Channel: %d\n", adc_reading_channel);

  gpio_it_register_interrupt(&button_addr, &button_interrupt_settings,
                             INTERRUPT_EDGE_FALLING,  // interrupt when button released
                             prv_interrupt_callback,  // private callback function
                             &adc_reading_channel);   // automatically cast to private void function

  while (true) {
    wait();
  }

  return 0;
}
