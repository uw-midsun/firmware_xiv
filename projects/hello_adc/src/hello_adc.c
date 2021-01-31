#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static void prv_log_pin_data(GpioAddress *button_addr, void *context);

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  GpioAddress button_addr = { GPIO_PORT_A, 6 };
  GpioAddress read_addr = { GPIO_PORT_B, 2 };
  // Setting pin value is not necessary
  GpioSettings pin_settings = { .direction = GPIO_DIR_IN, .alt_function = GPIO_ALTFN_ANALOG };
  gpio_init_pin(&button_addr, &pin_settings);
  gpio_init_pin(&read_addr, &pin_settings);
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(read_addr, true);
  InterruptSettings button_interrupt = { .type = INTERRUPT_TYPE_EVENT,
                                         .priority = INTERRUPT_PRIORITY_NORMAL };
  gpio_it_register_interrupt(&button_addr, &button_interrupt, INTERRUPT_EDGE_FALLING,
                             (GpioItCallback)prv_log_pin_data, &read_addr);
  while (true) {
  }
  return 0;
}

static void prv_log_pin_data(GpioAddress *button_addr, void *context) {
  GpioAddress *read_addr = context;
  uint16_t pin_data = 0;
  adc_read_converted_pin(*read_addr, &pin_data);
  LOG_DEBUG("Pin A6 data: %d\n", pin_data);
}
