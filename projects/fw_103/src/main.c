#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

static void prv_button_callback(const GpioAddress *address, void *context) {
  GpioAddress *adc_addr = context;
  uint16_t adc_reading = 0;

  if (adc_read_converted_pin(*adc_addr, &adc_reading) == STATUS_CODE_OK) {
    LOG_DEBUG("A6 ADC converted reading: %d\n", adc_reading);
  } else {
    LOG_DEBUG("Failed to read adc data");
  }
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  GpioAddress adc_addr = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };
  GpioAddress button_addr = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  GpioSettings adc_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_HIGH,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };

  InterruptSettings interrupt_settings = {
    .type = 0,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&adc_addr, &adc_settings);
  gpio_init_pin(&button_addr, &button_settings);

  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(adc_addr, true);

  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_callback, &adc_addr);

  while (true) {
    wait();
  }

  return 0;
}
