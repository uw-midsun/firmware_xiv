#include "adc.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

void prv_analog_read_a6(const GpioAddress *address, void *context) {
  uint16_t a6_data = 0;
  GpioAddress *a6_addr = context;

  if (adc_read_raw_pin(*a6_addr, &a6_data) == STATUS_CODE_OK) {
    LOG_DEBUG("A6 data: %d\n", a6_data);
  } else {
    LOG_DEBUG("Failed to read A6 data");
  }
}

int main(void) {
  GpioAddress a6_reading_addr = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };

  GpioSettings a6_reading_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  GpioAddress button_addr = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  InterruptSettings button_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  interrupt_init();
  gpio_init();
  gpio_it_init();
  gpio_init_pin(&a6_reading_addr, &a6_reading_settings);
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(a6_reading_addr, true);

  gpio_it_register_interrupt(&button_addr, &button_settings, INTERRUPT_EDGE_FALLING,
                             prv_analog_read_a6, &a6_reading_addr);

  while (true) {
    wait();
  }

  return 0;
}
