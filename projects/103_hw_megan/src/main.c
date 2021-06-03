#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

// logs the output of the ADC reading from pin A6
static void prv_log_adc_reading(const GpioAddress *address, void *context) {
  GpioAddress *adc_addr = context;
  uint16_t adc_data = 0;
  adc_read_converted_pin(*adc_addr, &adc_data);

  LOG_DEBUG("The ADC converted reading is: %d\n", adc_data);
}

int main() {
  interrupt_init();
  gpio_init();

  // gpio pin 6
  GpioAddress adc_addr = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  GpioSettings adc_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&adc_addr, &adc_settings);
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(adc_addr, true);

  // gpio button pin 2
  GpioAddress button_addr = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };

  gpio_init_pin(&button_addr, &button_settings);

  InterruptSettings interrupt_settings = {
    INTERRUPT_TYPE_INTERRUPT,
    INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_log_adc_reading, &adc_addr);

  while (true) {
    wait();
  }
}
