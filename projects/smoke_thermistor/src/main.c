#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// Toggle the following to select which data is logged
#define READ_DATA_RAW true
#define READ_DATA_CONVERTED false
#define TIME_BETWEEN_READS_IN_MILLIS 200

static GpioAddress s_pin_addresses[] = {
  { GPIO_PORT_A, 0 }, { GPIO_PORT_A, 1 }, { GPIO_PORT_A, 2 },
  { GPIO_PORT_A, 3 }, { GPIO_PORT_A, 4 }, { GPIO_PORT_A, 5 }
};

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GpioSettings settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };

  for (uint8_t pin = 0; pin < SIZEOF_ARRAY(s_pin_addresses); pin++) {
    gpio_init_pin(&s_pin_addresses[pin], &settings);
  }

  adc_init(ADC_MODE_SINGLE);

  for (uint8_t pin = 0; pin < SIZEOF_ARRAY(s_pin_addresses); pin++) {
    adc_set_channel_pin(s_pin_addresses[pin], true);
  }

  while (true) {
    uint16_t temp_data = 0;
    if (READ_DATA_RAW) {
      for (uint8_t reading = 0; reading < SIZEOF_ARRAY(s_pin_addresses); reading++) {
        adc_read_raw_pin(s_pin_addresses[reading], &temp_data);
        LOG_DEBUG("Temp %d raw data: %d\n", reading, temp_data);
      }
    }
    if (READ_DATA_CONVERTED) {
      for (uint8_t reading = 0; reading < SIZEOF_ARRAY(s_pin_addresses); reading++) {
        adc_read_converted_pin(s_pin_addresses[reading], &temp_data);
        LOG_DEBUG("Temp %d converted data: %d\n", reading, temp_data);
      }
    }
    delay_ms(TIME_BETWEEN_READS_IN_MILLIS);
  }
}
