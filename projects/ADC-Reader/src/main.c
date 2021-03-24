#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

static void prv_read_button(const GpioAddress *address, void *context) {
  AdcChannel *channel = context;
  uint16_t data_read = 0;

  adc_read_converted(*channel, &data_read);
  LOG_DEBUG("Data read and Converted : %d\n", data_read);

  gpio_it_trigger_interrupt(address);
}

int main(void) {
  // intializing
  interrupt_init();
  gpio_init();
  gpio_it_init();

  GpioAddress adc_reader_add = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress adc_button_add = { .port = GPIO_PORT_B, .pin = 2 };

  // settings
  GpioSettings adc_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_EVENT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&adc_reader_add, &adc_settings);
  gpio_init_pin(&adc_button_add, &adc_settings);

  adc_init(ADC_MODE_SINGLE);
  AdcChannel reader_channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_reader_add, &reader_channel);
  adc_set_channel(reader_channel, true);

  gpio_it_register_interrupt(&adc_button_add, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_read_button, &reader_channel);

  gpio_it_trigger_interrupt(&adc_button_add);

  while (true) {
    wait();
  }

  return 0;
}
