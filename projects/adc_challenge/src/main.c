#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

// reads from the specified address and logs the value
static void prv_read_from_adc_addr(const GpioAddress *address, void *context) {
  GpioAddress addr = {
    .port = address->port,  //
    .pin = address->pin     //
  };
  uint16_t adc_data = 0;
  adc_read_converted_pin(addr, &adc_data);

  LOG_DEBUG("ADC pin has value %d", adc_data);
}

int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress input_addr = { .port = GPIO_PORT_A, .pin = 6 };

  GpioSettings input_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };

  gpio_init_pin(&input_addr, &input_settings);
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(input_addr, true);

  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,         //
    .state = GPIO_STATE_LOW,          //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };
  gpio_init_pin(&button_addr, &button_settings);

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_read_from_adc_addr, &input_addr);

  while (true) {
    wait();
  }

  return 0;
}
