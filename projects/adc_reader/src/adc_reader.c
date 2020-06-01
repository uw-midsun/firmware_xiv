#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "interrupt_def.h"

typedef struct {
  AdcChannel channel;
  uint16_t data;
} AdcReader;

static void prv_button_pressed(const GpioAddress *address, void *context) {
  AdcReader *reader = context;
  adc_read_converted(reader->channel,&reader->data);
}

int main(void) {
  interrupt_init();
  gpio_init();
  gpio_it_init();

  GpioAddress reader_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings reader_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  GpioSettings button_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_NONE,
  };

  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };

  gpio_init_pin(&reader_addr, &reader_settings);
  gpio_init_pin(&button_addr, &button_settings);
  
  adc_init(ADC_MODE_SINGLE);
  AdcChannel reader_channel = NUM_ADC_CHANNELS;
  adc_get_channel(reader_addr, &reader_channel);
  adc_set_channel(reader_channel, true);

  AdcReader adc_reader = {
    .channel = reader_channel,
    .data = 0,
  };

  gpio_it_register_interrupt(&button_addr, &interrupt_settings,
                              INTERRUPT_EDGE_FALLING, 
                              prv_button_pressed, 
                              &adc_reader);

  while (true) {}
}
