#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "soft_timer.h"

// Packages context for callback function outside main
typedef struct Context_package {
  GpioAddress button_addr;
  InterruptSettings button_interrupt_settings;
  AdcChannel adc_reading_channel;
} Context_package;

static void prv_interrupt_callback(const GpioAddress *address, void *context) {
  // casting void* back for use outside main()
  Context_package *p_package = context;

  // Unpacking context package structure
  GpioAddress button_addr = p_package->button_addr;
  InterruptSettings button_interrupt_settings = p_package->button_interrupt_settings;
  AdcChannel adc_reading_channel = p_package->adc_reading_channel;

  // read and print adc reading data
  uint16_t adc_reading_data = 0;
  adc_read_converted(adc_reading_channel, &adc_reading_data);
  LOG_DEBUG("ADC Reading Data: %d\n", adc_reading_data);

  // restart interrupt watch
  gpio_it_register_interrupt(&button_addr, &button_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_interrupt_callback, p_package);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress adc_reading_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings adc_reading_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_HIGH,
    GPIO_RES_PULLDOWN,
    GPIO_ALTFN_ANALOG,
  };

  InterruptSettings button_interrupt_settings = {
    INTERRUPT_TYPE_INTERRUPT,
    INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&adc_reading_addr, &adc_reading_settings);
  gpio_init_pin(&button_addr, &button_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel adc_reading_channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_reading_addr, &adc_reading_channel);
  adc_set_channel(adc_reading_channel, true);

  LOG_DEBUG("ADC Reading Channel: %d\n", adc_reading_channel);

  Context_package context = { button_addr, button_interrupt_settings, adc_reading_channel };

  gpio_it_register_interrupt(&button_addr, &button_interrupt_settings,
                             INTERRUPT_EDGE_FALLING,  // interrupt when button released
                             prv_interrupt_callback,  // private callback function
                             &context);               // automatically cast to private void function

  while (true) {
    wait();
  }

  return 0;
}
