#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// interrupt call back function
void interrupt_callback(const GpioAddress *address, void *context) {
  GpioAddress *ADC_address = context;

  uint16_t ADC_reading = 0;
  if (adc_read_raw_pin(*ADC_address, &ADC_reading) == STATUS_CODE_OK) {
    LOG_DEBUG("Output: &d\n", ADC_reading);
  } else {
    LOG_DEBUG("Failed to read.");
  }
}

int main(void) {
  gpio_it_init();
  soft_timer_init();
  gpio_init();

  GpioAddress ADC_addr = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  GpioAddress button_addr = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  GpioSettings pot_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&ADC_addr, &pot_settings);
  gpio_init_pin(&button_addr, &pot_settings);
  adc_set_channel_pin(ADC_addr, true);

  // waits for GPIO to fall from high to low (button released) as the button is active-high
  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             interrupt_callback, &ADC_addr);

  while (true) {
    wait();
  }
  return 0;
}
