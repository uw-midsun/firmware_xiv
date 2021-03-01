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
#include "wait.h"

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  GpioAddress *adc_read = (GpioAddress *)context;
  uint16_t adc_value = 0;
  adc_read_converted_pin(*adc_read, &adc_value);
  LOG_DEBUG("Button released! ADC_Value: %d\n", adc_value);
}

static InterruptSettings interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static GpioSettings adc_settings = {
  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_ANALOG,
};

static GpioSettings button_settings = {
  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_LOW,
  .resistor = GPIO_RES_NONE,
  .alt_function = GPIO_ALTFN_NONE,
};

static GpioAddress adc_reading_addr = { .port = GPIO_PORT_A, .pin = 6 };
static GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  gpio_init_pin(&adc_reading_addr, &adc_settings);
  gpio_init_pin(&button_addr, &button_settings);

  adc_init(ADC_MODE_SINGLE);
  AdcChannel adc_channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_reading_addr, &adc_channel);
  adc_set_channel(adc_channel, true);

  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &adc_reading_addr);

  while (true) {
    wait();
  }
  return 0;
}
