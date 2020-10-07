#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

#define READ_DATA "raw"  // "raw" or "converted"

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GpioAddress temp1_addr = { .port = GPIO_PORT_A, .pin = 0 };
  GpioAddress temp2_addr = { .port = GPIO_PORT_A, .pin = 1 };
  GpioAddress temp3_addr = { .port = GPIO_PORT_A, .pin = 2 };
  GpioAddress temp4_addr = { .port = GPIO_PORT_A, .pin = 3 };
  GpioAddress temp5_addr = { .port = GPIO_PORT_A, .pin = 4 };
  GpioAddress temp6_addr = { .port = GPIO_PORT_A, .pin = 5 };

  GpioSettings settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };

  gpio_init_pin(&temp1_addr, &settings);
  gpio_init_pin(&temp2_addr, &settings);
  gpio_init_pin(&temp3_addr, &settings);
  gpio_init_pin(&temp4_addr, &settings);
  gpio_init_pin(&temp5_addr, &settings);
  gpio_init_pin(&temp6_addr, &settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel temp1_channel = NUM_ADC_CHANNELS;
  adc_get_channel(temp1_addr, &temp1_channel);
  adc_set_channel(temp1_channel, true);
  LOG_DEBUG("Temp1 ADC Channel: %d\n", temp1_channel);

  AdcChannel temp2_channel = NUM_ADC_CHANNELS;
  adc_get_channel(temp2_addr, &temp2_channel);
  adc_set_channel(temp2_channel, true);
  LOG_DEBUG("Temp2 ADC Channel: %d\n", temp2_channel);

  AdcChannel temp3_channel = NUM_ADC_CHANNELS;
  adc_get_channel(temp3_addr, &temp3_channel);
  adc_set_channel(temp3_channel, true);
  LOG_DEBUG("Temp3 ADC Channel: %d\n", temp3_channel);

  AdcChannel temp4_channel = NUM_ADC_CHANNELS;
  adc_get_channel(temp4_addr, &temp4_channel);
  adc_set_channel(temp4_channel, true);
  LOG_DEBUG("Temp4 ADC Channel: %d\n", temp4_channel);

  AdcChannel temp5_channel = NUM_ADC_CHANNELS;
  adc_get_channel(temp5_addr, &temp5_channel);
  adc_set_channel(temp5_channel, true);
  LOG_DEBUG("Temp5 ADC Channel: %d\n", temp5_channel);

  AdcChannel temp6_channel = NUM_ADC_CHANNELS;
  adc_get_channel(temp6_addr, &temp6_channel);
  adc_set_channel(temp6_channel, true);
  LOG_DEBUG("Temp6 ADC Channel: %d\n", temp6_channel);

  while (true) {
    uint16_t temp_data = 0;
    if (READ_DATA == "raw") {
      adc_read_raw(temp1_channel, &temp_data);
      LOG_DEBUG("Temp1 data: %d\n", temp_data);
      adc_read_raw(temp2_channel, &temp_data);
      LOG_DEBUG("Temp2 data: %d\n", temp_data);
      adc_read_raw(temp3_channel, &temp_data);
      LOG_DEBUG("Temp3 data: %d\n", temp_data);
      adc_read_raw(temp4_channel, &temp_data);
      LOG_DEBUG("Temp4 data: %d\n", temp_data);
      adc_read_raw(temp5_channel, &temp_data);
      LOG_DEBUG("Temp5 data: %d\n", temp_data);
      adc_read_raw(temp6_channel, &temp_data);
      LOG_DEBUG("Temp6 data: %d\n", temp_data);
    } else if (READ_DATA == "converted") {
      adc_read_converted(temp1_channel, &temp_data);
      LOG_DEBUG("Temp1 data: %d\n", temp_data);
      adc_read_converted(temp2_channel, &temp_data);
      LOG_DEBUG("Temp2 data: %d\n", temp_data);
      adc_read_converted(temp3_channel, &temp_data);
      LOG_DEBUG("Temp3 data: %d\n", temp_data);
      adc_read_converted(temp4_channel, &temp_data);
      LOG_DEBUG("Temp4 data: %d\n", temp_data);
      adc_read_converted(temp5_channel, &temp_data);
      LOG_DEBUG("Temp5 data: %d\n", temp_data);
      adc_read_converted(temp6_channel, &temp_data);
      LOG_DEBUG("Temp6 data: %d\n", temp_data);
    }
    delay_ms(200);
  }
}
