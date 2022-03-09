#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"


static void prv_button_interrupt_callback(const GpioAddress *potentiometer_addr, void *context) {
  button_data = adc_read_converted_pin(GpioAddress address, uint16_t *reading);
  LOG_DEBUG("button data: %d\n", );
};

int main(void) {
  gpio_init();
  gpio_it_init();
  adc_init(ADC_MODE_SINGLE);

  GpioAddress potentiometer_addr = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  GpioSettings pot_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&potentiometer_addr, &pot_settings);

  InterruptSettings button_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_HIGH,
  };

  adc_set_channel_pin(potentiometer_addr, true);

  gpio_it_register_interrupt(&potentiometer_addr, &button_interrupt_settings, INTERRUPT_EDGE_RISING,
                             &prv_button_interrupt_callback, NULL);

  while (true) {
    wait();
  }
}
