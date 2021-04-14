#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_callback(const GpioAddress *address, void *context) {
  GpioAddress *A6 = context;
  uint16_t A6_data = 0;
  adc_read_converted_pin(*A6, &A6_data);
  LOG_DEBUG("%d\n", A6_data);
}

int main(void) {
  gpio_init();
  gpio_it_init();
  interrupt_init();

  GpioAddress button_pin = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  GpioAddress adc_reading = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  GpioSettings button_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  GpioSettings adc_reading_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(adc_reading, true);

  InterruptSettings settings = {
    INTERRUPT_TYPE_INTERRUPT,
    INTERRUPT_PRIORITY_NORMAL,
  };

  InterruptEdge edge = INTERRUPT_EDGE_FALLING;

  gpio_it_register_interrupt(&button_pin, &settings, edge, prv_callback, &adc_reading);

  while (true) {
    wait();
  }

  return 0;
}
