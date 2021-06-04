#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GpioAddress potentiometer_addr = {
    .port = GPIO_PORT_A,
    .pin = 0,
  };

  GpioSettings pot_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&potentiometer_addr, &pot_settings);
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(potentiometer_addr, true);

  while (true) {
    uint16_t potentiometer_data = 0;
    adc_read_raw_pin(potentiometer_addr, &potentiometer_data);
    LOG_DEBUG("potentiometer data: %d\n", potentiometer_data);
    delay_ms(200);
  }

  return 0;
}
