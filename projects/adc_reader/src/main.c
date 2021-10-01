#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "gpio_it.h"


int main(void){
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GpioAddress reading_addr = {.port = GPIO_PORT_A, .pin = A6};

  GpioAddress button_addr = {.port = GPIO_PORT_A, .pin = B2};

  GpioSettings reading_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&reading_addr, &reading_settings);
  gpio_init_pin(&button_addr, &button_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel reading_channel = NUM_ADC_CHANNELS;
  AdcChannel button_channel = NUM_ADC_CHANNELS;

  adc_get_channel_pin(reading_addr, true);
  // adc_set_channel(reading_channel, true);
  // adc_set_channel(ADC_CHANNEL_TEMP, true);

  adc_get_channel_pin(button_addr, true);
//  adc_set_channel(button_channel, true); 
//  adc_set_channel(ADC_CHANNEL_TEMP, true);

  LOG_DEBUG("Reading ADC Channel: %d\n", reading_channel);

  while(true){
    uint8_t button_data= 0;
    uint8_t reading_data = 0;
    if (adc_read_raw_pin(button_addr, &button_data) == STATUS_CODE_OK){
      float adc_reading = adc_read_converted_pin(reading_addr, &reading_data);
      LOG_DEBUG("%d", adc_reading);
      delay(200);
    }
  }

}
