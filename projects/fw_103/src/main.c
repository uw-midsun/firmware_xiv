#include <stdint.h>  // for integer types

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"  // interrupts are required for soft timers
#include "log.h"        // for printing
#include "wait.h"       // for wait function

static GpioAddress s_adc_addr = {
  .port = GPIO_PORT_A,
  .pin = 6,
};

static void prv_read_adc(const GpioAddress *btn_addr_ptr, void *context) {
  uint16_t adc_data = 0;
  if (adc_read_converted_pin(s_adc_addr, &adc_data) == STATUS_CODE_OK) {
    LOG_DEBUG("ADC data: %d\n", adc_data);
  } else {
    LOG_DEBUG("Failed to read adc data");
  }
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  GpioAddress btn_addr = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };
  GpioSettings adc_gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  GpioSettings btn_gpio_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
  };

  gpio_init_pin(&s_adc_addr, &adc_gpio_settings);
  gpio_init_pin(&btn_addr, &btn_gpio_settings);
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(s_adc_addr, true);

  InterruptSettings settings = { .priority = INTERRUPT_PRIORITY_NORMAL,
                                 .type = INTERRUPT_TYPE_INTERRUPT };
  gpio_it_register_interrupt(&btn_addr, &settings, INTERRUPT_EDGE_FALLING, prv_read_adc, NULL);

  while (true) {
    wait();  // wait till interrupt is triggered
  }
  return 0;
}
