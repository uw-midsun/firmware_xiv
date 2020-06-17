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

  GpioAddress potentiometer_addr = { .port = GPIO_PORT_A, .pin = 0 };

  GpioSettings pot_settings = {
    GPIO_DIR_IN,        //
    GPIO_STATE_LOW,     //
    GPIO_RES_NONE,      //
    GPIO_ALTFN_ANALOG,  //
  };

  gpio_init_pin(&potentiometer_addr, &pot_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel pot_channel = NUM_ADC_CHANNELS;

  adc_get_channel(potentiometer_addr, &pot_channel);
  adc_set_channel(pot_channel, true);
  adc_set_channel(ADC_CHANNEL_TEMP, true);

  LOG_DEBUG("Potentiometer ADC Channel: %d\n", pot_channel);

  while (true) {
    uint16_t potentiometer_data = 0;
    uint16_t temperature_data = 0;
    adc_read_raw(pot_channel, &potentiometer_data);
    adc_read_raw(ADC_CHANNEL_TEMP, &temperature_data);
    LOG_DEBUG("potentiometer data: %d\n", potentiometer_data);
    delay_ms(200);
  }

  return 0;
}
