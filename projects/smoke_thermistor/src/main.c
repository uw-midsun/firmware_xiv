#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Toggle the following to select which data is logged
#define READ_DATA_RAW true
#define READ_DATA_CONVERTED false
#define NUM_ADC_PINS 6
#define TIME_BETWEEN_READS_IN_MILLIS 200

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GpioAddress pin_addresses[NUM_ADC_PINS];

  for (uint8_t pin = 0; pin < NUM_ADC_PINS; pin++) {
    pin_addresses[pin] = {.port = GPIO_PORT_A, .pin = pin }
  }

  GpioSettings settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };

  for (uint8_t pin = 0; pin < NUM_ADC_PINS; pin++) {
    gpio_init_pin(&pin_addresses[pin], &settings);
  }

  adc_init(ADC_MODE_SINGLE);

  for (uint8_t pin = 0; pin < NUM_ADC_PINS; pin++) {
    adc_set_channel_pin(pin_addresses[pin], true);
  }

  while (true) {
    uint16_t temp_data = 0;
    if (READ_DATA_RAW) {
      for (uint8_t reading = 0; reading < NUM_ADC_PINS; reading++) {
        adc_read_raw_pin(pin_addresses[reading], &temp_data);
        LOG_DEBUG("Temp %d raw data: %d\n", reading, temp_data)
      }
    }
    if (READ_DATA_CONVERTED) {
      for (uint8_t reading = 0; reading < NUM_ADC_PINS; reading++) {
        adc_read_converted_pin(pin_addresses[reading], &temp_data);
        LOG_DEBUG("Temp %d converted data: %d\n", reading, temp_data)
      }
    }
    delay_ms(TIME_BETWEEN_READS_IN_MILLIS);
  }
}
