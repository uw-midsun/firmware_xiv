#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

// A6 input, B2 input, log output

typedef enum { ADC_IN = 0, BTN_IN, NUM_ADDRESSES } Addresses;

// callback func, takes in the channel and reads ADC data from channel into local var
// then logs the var data
static void prv_btn_interrupt_handler(const GpioAddress *address, void *context) {
  AdcChannel *in_channel = (AdcChannel *)context;
  uint16_t in_data = 0;

  adc_read_converted(*in_channel, &in_data);
  LOG_DEBUG("Input ADC Data: %d\n", in_data);
}

int main(void) {
  // initialize libraries
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  // assign ports and pins for ADC input and Button input
  GpioAddress adc_button_addresses[] = {
    [ADC_IN] = { .port = GPIO_PORT_A, .pin = 6 },  //
    [BTN_IN] = { .port = GPIO_PORT_B, .pin = 2 },  //
  };

  // initialize settings for ADC gpio input
  GpioSettings adc_settings = {
    .direction = GPIO_DIR_IN,           //
    .state = GPIO_STATE_LOW,            //
    .resistor = GPIO_RES_NONE,          //
    .alt_function = GPIO_ALTFN_ANALOG,  //
  };

  // initialize settings for button interrupt
  InterruptSettings btn_interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  // initialize adc
  gpio_init_pin(&adc_button_addresses[ADC_IN], &adc_settings);

  adc_init(ADC_MODE_SINGLE);

  // set channel to default (0), then get the channel using the get func with the port/pin
  // set input cahnnel to true, enable it
  AdcChannel in_channel = NUM_ADC_CHANNELS;

  adc_get_channel(adc_button_addresses[ADC_IN], &in_channel);
  adc_set_channel(in_channel, true);

  // initialize the button interrupt with btn info and callback func
  gpio_it_register_interrupt(&adc_button_addresses[BTN_IN], &btn_interrupt_settings,
                             INTERRUPT_EDGE_FALLING, prv_btn_interrupt_handler, &in_channel);

  while (true) {
  }

  return 0;
}
