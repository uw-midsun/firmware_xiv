#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"
#include "log.h"
#include "wait.h"

// LOG_DEBUG the adc converted value of context pin when called (context is A6 pin)
void log_adc_a6(const GpioAddress *address, void *context) {
  GpioAddress *pin_a6_addr = context;
  uint16_t pin_a6_data = 0;
  adc_read_converted_pin(*pin_a6_addr, &pin_a6_data);
  LOG_DEBUG("adc converted pin A6 data: %d\n", pin_a6_data);
}

int main(void) {
  interrupt_init();
  gpio_init();
  adc_init(ADC_MODE_SINGLE);

  // output gpio pin A6
  GpioAddress analog_pin_a6_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioSettings analog_pin_a6_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };
  gpio_init_pin(&analog_pin_a6_addr, &analog_pin_a6_settings);

  // button gpio pin
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };
  GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };
  gpio_init_pin(&button_addr, &button_settings);

  InterruptSettings interrupt_settings = {
    INTERRUPT_TYPE_INTERRUPT,
    INTERRUPT_PRIORITY_HIGH,
  };

  // calls log_adc_output on the falling edge of button press
  gpio_it_register_interrupt(&button_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING, log_adc_a6,
                             &analog_pin_a6_addr);

  while (true) {
    wait();
  }
}
