#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

static InterruptSettings interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  uint16_t adc_data = 0;
  if (adc_read_raw_pin(*address, &adc_data) == STATUS_CODE_OK) {
    LOG_DEBUG("adc data: %d\n", adc_data);
  }
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  GpioAddress adc_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress btn_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings btn_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_NONE };

  GpioSettings adc_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  gpio_init_pin(&adc_addr, &adc_settings);
  gpio_init_pin(&btn_addr, &btn_settings);
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(adc_addr, true);

  gpio_it_register_interrupt(&btn_addr, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             prv_button_interrupt_handler, NULL);

  while (true) {
    wait();
  }
}
