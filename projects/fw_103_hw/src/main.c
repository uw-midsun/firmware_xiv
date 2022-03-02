#include <stdbool.h>
#include <stdio.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

/*
 * Create a project that reads an ADC converted reading from pin A6
 * whenever a button connected to pin B2 is pressed and logs the output.
 * The adc reading should trigger whenever the button is released,
 * not initially when the button is pressed.
 * Follow the same submission procedures as the firmware 102 homework,
 * except the suffix of the branch should be fw_103.

Hints:
assume the button is active-high

register an interrupt with the appropriate InterruptEdge settings (google this or ask someone / your
lead)

 */

int main(void) {
  gpio_init();
  gpio_it_init();
  adc_init(ADC_MODE_SINGLE);

  GpioAddress button_addr = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  GpioSettings pot_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&button_addr, &pot_settings);

  InterruptSettings button_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_HIGH,
  };

  adc_set_channel_pin(button_addr, true);

  gpio_it_get_edge(&button_addr, INTERRUPT_EDGE_RISING);

  uint16_t button_data = 0;
  void prv_button_interrupt_callback(const GpioAddress *button_addr, void *context) {
    LOG_DEBUG("button data: %d\n", button_data);
  };

  gpio_it_register_interrupt(&button_addr, &button_interrupt_settings, INTERRUPT_EDGE_RISING,
                             &prv_button_interrupt_callback, NULL);

  while (true) {
    wait();
  }
}
