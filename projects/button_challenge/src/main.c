#include <stdint.h>
#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

// Addresses
static const GpioAddress button_address = { .port = GPIO_PORT_B, .pin = 2 };

static const GpioAddress adc_address = { .port = GPIO_PORT_A, .pin = 6 };

// Settings
static GpioSettings button_settings = { .direction = GPIO_DIR_IN,
                                        .state = GPIO_STATE_HIGH,
                                        .alt_function = GPIO_ALTFN_NONE,
                                        .resistor = GPIO_RES_PULLDOWN };

static GpioSettings adc_settings = { .direction = GPIO_DIR_OUT,
                                     .state = GPIO_STATE_LOW,
                                     .alt_function = GPIO_ALTFN_ANALOG,
                                     .resistor = GPIO_RES_NONE };

static InterruptSettings interrupt_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                                .priority = INTERRUPT_PRIORITY_NORMAL };

// Callback function
static void prv_adc_callback(const GpioAddress *address, void *context) {
  AdcChannel *adc_channel = context;
  uint16_t adc_data = 0;

  adc_read_raw(*adc_channel, &adc_data);
  LOG_DEBUG("ADC data: %d\n", adc_data);
}

// Main
int main() {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  gpio_init_pin(&button_address, &button_settings);
  gpio_init_pin(&adc_address, &adc_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel adc_channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_address, &adc_channel);
  adc_set_channel(adc_channel, true);
  adc_set_channel(ADC_CHANNEL_TEMP, true);

  gpio_it_register_interrupt(&button_address, &interrupt_settings, INTERRUPT_EDGE_RISING,
                             prv_adc_callback, &adc_address);

  while (true) {
  }

  return 0;
}
