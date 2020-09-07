#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// used for storage
typedef struct AdcReader {
  uint16_t reading;
  AdcChannel channel;
} AdcReader;

// callback when button is released (falling edge)
void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  AdcReader *storage = context;
  adc_read_converted(storage->channel, &storage->reading);
}

int main() {
  // required initializations
  interrupt_init();
  gpio_init();
  gpio_it_init();

  // address for whatever the adc is reading
  GpioAddress adc_address = { .port = GPIO_PORT_A, .pin = 6 };

  // settings for the adc
  GpioSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .alt_function = GPIO_ALTFN_ANALOG,
                                .resistor = GPIO_RES_NONE };

  // address for button
  GpioAddress button_address = { .port = GPIO_PORT_B, .pin = 2 };

  // settings for buttons
  GpioSettings button_settings = { .direction = GPIO_DIR_IN,
                                   .state = GPIO_STATE_LOW,
                                   .alt_function = GPIO_ALTFN_NONE,
                                   .resistor = GPIO_RES_NONE };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  // initialize the gpio pins
  gpio_init_pin(&adc_address, &adc_settings);
  gpio_init_pin(&button_address, &button_settings);
  adc_init(ADC_MODE_SINGLE);  // converts only 1 channel

  AdcChannel adc_channel = NUM_ADC_CHANNELS;
  adc_get_channel(adc_address, &adc_channel);
  adc_set_channel(adc_channel, true);

  // initialize the struct
  AdcReader storage = { .reading = 0, .channel = adc_channel };

  // if interrupt is triggered go the callback function
  // since we want when the button is released, using INTERRUPT_EDGE_FALLING
  gpio_it_register_interrupt(&button_address, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &storage);
  while (true) {
    wait();
  }
  return 0;
}
