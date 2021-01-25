#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define BUTTON_PIN 178
#define READ_FROM_PIN 166

GpioItCallback prv_log_pin_data(GpioAddress *button_addr, void *context);

int main() {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  GpioAddress button_addr = { GPIO_PORT_A, BUTTON_PIN };
  GpioAddress read_addr = { GPIO_PORT_A, READ_FROM_PIN };
  // Setting pin value is not necessary
  GpioSettings null_settings = {};
  gpio_init_pin(&button_addr, &null_settings);
  gpio_init_pin(&read_addr, &null_settings);
  adc_init(ADC_MODE_SINGLE);
  InterruptSettings button_interrupt = { INTERRUPT_TYPE_EVENT, INTERRUPT_PRIORITY_NORMAL };
  gpio_it_register_interrupt(&button_addr, &button_interrupt, INTERRUPT_EDGE_FALLING,
                             (GpioItCallback)prv_log_pin_data, &read_addr);
  while (true) {
  }
  return 0;
}

GpioItCallback prv_log_pin_data(GpioAddress *button_addr, void *context) {
  GpioAddress *read_addr = context;
  uint16_t pin_data = 0;
  adc_read_converted_pin(*read_addr, &pin_data);
  LOG_DEBUG("Pin A6 data: %d\n", pin_data);
  return NULL;
}
