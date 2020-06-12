#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h

typedef struct AdcData {
  uint16_t  reading;
  AdcChannel channel;
} AdcData;

void prv_button_interrupt_handler(const GpioAddress *address, void *context){
  AdcData* temp_store = context;
  adc_read_converted(temp_store->channel,&temp_store->reading);
  LOG_DEBUG("adc reading %d\n", temp_store->reading);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAdress button_address = { .port = GPIO_PORT_B, .pin = 2 };

  GpioAddress adc_address = {.port = GPIO_PORT_A, pin = 6 };

  GpioSettings adc_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG
  };

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .alt_function = GPIO_ALTFN_NONE,
    .resistor = GPIO_RES_PULLDOWN
  };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL
  };

  gpio_init_pin(&adc_address, &adc_settings);
  gpio_init_pin(&button_address, &button_settings);
  adc_init(ADC_MODE_SINGLE);

  AdcChannel adc_channel = NUM_ADC_CHANNELS;
  adc_get_channel(adc_address, &adc_channel);
  adc_set_channel(adc_channel, true);

  AdcData adc_channel_and_reading = {.reading = 0, .channel = adc_channel };

  gpio_it_register_interrupt(&button_address, &interrupt_settings,
                             INTERRUPT_EDGE_FALLING, prv_button_interrupt_handler,
                             &adc_channel_and_reading);

  while (true) {
  }

  return 0;
}
