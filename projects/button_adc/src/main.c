#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_button_press_callback(const GpioAddress *address, void *context) {
  GpioAddress *analog_pin = context;
  uint16_t analog_pin_data = 0;
  adc_read_converted_pin(*analog_pin, &analog_pin_data);
  LOG_DEBUG("Analog Pin data: %d\n", analog_pin_data);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress analog_pin = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings pin_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };
  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,  //
    .state = GPIO_STATE_LOW,   //
  };
  InterruptSettings it_setting = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };
  gpio_init_pin(&analog_pin, &pin_settings);
  gpio_init_pin(&button, &button_settings);

  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(analog_pin, true);
  LOG_DEBUG("Analog Pin: %c%d\n", analog_pin.port+65, analog_pin.pin);
  LOG_DEBUG("Button Pin: %c%d\n", button.port+65, button.pin);

  gpio_it_register_interrupt(&button, &it_setting, INTERRUPT_EDGE_FALLING, prv_button_press_callback,
                             &analog_pin);

  while (true) {
    wait();
  }
  return 0;
}
