#include <stdbool.h>
#include "gpio.h"
#include "gpio_it.h"
#include "inc/adc.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

// Interrupt callback
static void prv_adc_callback(GpioAddress *address, void *context) {
  GpioAddress *adc_address = context;

  uint16_t adc_reading = 0;
  adc_read_raw_pin(*adc_address, &adc_reading);

  LOG_DEBUG("Read ADC value");
}

int main(void) {
  // Address Setups
  GpioAddress adc_address = {
    .port = GPIO_PORT_A,  //
    .pin = 6,             //
  };

  GpioAddress button_address = {
    .port = GPIO_PORT_B,  //
    .pin = 2,             //
  };

  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,          //
    .state = GPIO_STATE_HIGH,           //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };

  InterruptSettings interrupt_settings = {
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
    .type = INTERRUPT_TYPE_INTERRUPT,       //
  };

  // Library Inits
  gpio_init();
  gpio_it_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);

  // Pin Inits
  gpio_init_pin(&adc_address, &gpio_settings);
  gpio_init_pin(&button_address, &gpio_settings);

  adc_set_channel_pin(adc_address, true);

  // Interrupt Code
  gpio_it_register_interrupt(&button_address, &interrupt_settings, INTERRUPT_EDGE_RISING_FALLING,
                             prv_adc_callback, &adc_address);

  // Infinite loop
  while (true) {
    // Wait for interrupts
    wait();
  }

  return 0;
}
