// Create a project that reads an ADC converted reading from pin A6 whenever a button connected to
// pin B2 and logs the output. The adc reading should trigger whenever the button is released, not
// initially when the button is pressed. Follow the same submission procedures as the firmware 102
// homework, except the suffix of the branch should be fw_103.

// Hints:
// assume the button is active-high
// register an interrupt with the appropriate InterruptEdge settings (google this or ask someone /
// your lead)
#include <stdbool.h>
#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"

void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  uint16_t adc_pin_data = 0;
  GpioAddress *adc_pin_addr = (GpioAddress *)context;
  adc_read_raw_pin(*adc_pin_addr, &adc_pin_data);
  LOG_DEBUG("Raw data: %d\n", adc_pin_data);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();
  adc_init(ADC_MODE_SINGLE);

  // button setup
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };
  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
  };
  InterruptSettings button_int_settings = {
    .priority = INTERRUPT_PRIORITY_HIGH,  //
    .type = INTERRUPT_TYPE_INTERRUPT      //
  };

  gpio_init_pin(&button_addr, &button_settings);

  // ADC setup
  GpioAddress adc_pin_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioSettings adc_pin_settings = {
    .direction = GPIO_DIR_IN, .state = GPIO_STATE_LOW, .alt_function = GPIO_ALTFN_ANALOG
  };
  gpio_init_pin(&adc_pin_addr, &adc_pin_settings);
  adc_set_channel_pin(adc_pin_addr, true);

  // interrupt setup
  gpio_it_register_interrupt(&button_addr, &button_int_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &adc_pin_addr);

  while (true) {
  }

  return 0;
}
