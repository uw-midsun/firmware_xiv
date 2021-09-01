#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

// testing git hook
static void prv_button_interrupt_handler(const GpioAddress *adc_address, void *context) {
  GpioAddress *adc_position = context;
  uint16_t adc_data = 0;
  StatusCode adc_converted_data = adc_read_converted_pin(*adc_position, &adc_data);
  if (adc_converted_data == STATUS_CODE_OK) {
    LOG_DEBUG("ADC reading is: %d\n", adc_data);
  } else {
    LOG_DEBUG("an error has occured.\n");
  }
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  // Button address
  GpioAddress button_address = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  // Button setting
  GpioSettings button_setting = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_NONE,
  };

  // Adc address
  GpioAddress adc_address = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  // Adc setting
  GpioSettings adc_setting = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .alt_function = GPIO_ALTFN_ANALOG,
    .resistor = GPIO_RES_NONE,
  };

  static InterruptSettings s_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&button_address, &button_setting);
  gpio_init_pin(&adc_address, &adc_setting);
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(adc_address, true);
  gpio_it_register_interrupt(&button_address, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, &adc_address);
  while (true) {
    wait();
  }

  return 0;
}
