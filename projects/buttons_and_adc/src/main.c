
#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

static GpioAddress s_adc_pin_address = { .port = GPIO_PORT_A, .pin = 6 };
static GpioAddress s_button_address = { .port = GPIO_PORT_B, .pin = 2 };

static GpioSettings s_button_settings = {
  .direction = GPIO_DIR_IN,         //
  .resistor = GPIO_RES_NONE,        //
  .alt_function = GPIO_ALTFN_NONE,  //
};

static GpioSettings s_adc_pin_settings = {
  .direction = GPIO_DIR_IN,           //
  .resistor = GPIO_RES_NONE,          //
  .alt_function = GPIO_ALTFN_ANALOG,  //
};

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,       //
  .priority = INTERRUPT_PRIORITY_NORMAL,  //
};

static void prv_callback(const GpioAddress *address, void *context) {
  uint16_t reading = 0;
  GpioAddress *s_adc_pin_address = context;

  adc_read_converted_pin(*s_adc_pin_address, &reading);
  LOG_DEBUG("ADC Reading: %i\n", reading);
}

int main() {
  gpio_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);

  gpio_init_pin(&s_adc_pin_address, &s_adc_pin_settings);
  gpio_init_pin(&s_button_address, &s_button_settings);

  adc_set_channel_pin(s_adc_pin_address, true);

  gpio_it_register_interrupt(&s_button_address,       //
                             &s_interrupt_settings,   //
                             INTERRUPT_EDGE_FALLING,  //
                             prv_callback,            //
                             &s_adc_pin_address);

  while (true) {
  }

  return 0;
}
