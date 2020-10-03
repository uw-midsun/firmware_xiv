#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_adc_callback(const GpioAddress *address, void *context) {
  AdcChannel *analogChannel = context;
  uint16_t analogPinData = 0;
  adc_read_raw(*analogChannel, &analogPinData);
  LOG_DEBUG("Analog Pin data: %d\n", analogPinData);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioAddress analogPin = { .port = GPIO_PORT_A, .pin = 6 };
  GpioAddress button = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings pinSettings = {
    GPIO_DIR_IN,        //
    GPIO_STATE_LOW,     //
    GPIO_RES_NONE,      //
    GPIO_ALTFN_ANALOG,  //
  };
  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,  //
    .state = GPIO_STATE_LOW,   //
  };
  InterruptSettings itSetting = {
    INTERRUPT_TYPE_INTERRUPT,   //
    INTERRUPT_PRIORITY_NORMAL,  //
  };
  gpio_init_pin(&analogPin, &pinSettings);
  gpio_init_pin(&button, &button_settings);

  adc_init(ADC_MODE_SINGLE);

  AdcChannel analogChannel = NUM_ADC_CHANNELS;

  adc_get_channel(analogPin, &analogChannel);
  adc_set_channel(analogChannel, true);

  LOG_DEBUG("ADC Channel: %d\n", analogChannel);

  gpio_it_register_interrupt(&button, &itSetting, INTERRUPT_EDGE_FALLING, prv_adc_callback,
                             &analogChannel);

  while (true) {
    wait();
  }
  return 0;
}
