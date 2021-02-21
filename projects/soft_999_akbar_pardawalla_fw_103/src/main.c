#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

void adc_callback(AdcChannel addr, void *context);

void bttn_callback(const GpioAddress *address, void *context) {
  AdcChannel *channel = context;
  adc_register_callback(*channel, adc_callback, address);
}

void adc_callback(AdcChannel addr, void *context) {
  uint16_t reading = 0;
  GpioAddress *address = context;

  adc_read_converted(addr, &reading);

  LOG_DEBUG("Converted Logic: %d", reading);

  gpio_it_trigger_interrupt(address);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  // Initialize the settings for adc

  GpioAddress adc_addr = { .port = GPIO_PORT_A, .pin = 6 };

  GpioSettings adc_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&adc_addr, &adc_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_addr, &channel);
  adc_set_channel(channel, true);

  GpioAddress bttn_addr = { .port = GPIO_PORT_B, .pin = 2 };

  InterruptSettings event_settings = {
    .type = INTERRUPT_TYPE_EVENT,           //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  gpio_it_register_interrupt(&bttn_addr, &event_settings, INTERRUPT_EDGE_RISING, bttn_callback,
                             &channel);

  gpio_it_trigger_interrupt(&bttn_addr);

  while (true) {
  }

  return 0;
}
