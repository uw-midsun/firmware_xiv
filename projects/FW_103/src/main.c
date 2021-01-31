/*
Create a project that reads an ADC converted reading from pin A6 whenever a button connected to pin
B2 and logs the output. The adc reading should trigger whenever the button is released, not
initially when the button is pressed. Follow the same submission procedures as the firmware 102
homework, except the suffix of the branch should be fw_103.

Hints:

- assume the button is active-high

- register an interrupt with the appropriate InterruptEdge settings (google this or ask someone /
your lead)
*/

// DATASHEET USED: https://www.st.com/resource/en/datasheet/dm00039193.pdf

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"
#include "wait.h"

static void prv_get_adc_callback(const GpioAddress *address, void *context);

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  // Set up the ADC
  GpioAddress adc_addr = { .port = GPIO_PORT_A, .pin = 6 };

  GpioSettings adc_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&adc_addr, &adc_settings);
  adc_init(ADC_MODE_SINGLE);

  // Set up the button
  GpioAddress button_addr = { .port = GPIO_PORT_B, .pin = 2 };

  GpioSettings button_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_HIGH,
    .resistor = GPIO_RES_NONE,
    .alt_function =
        GPIO_ALTFN_NONE,  // I believe Pin B2 can only have the 3rd alternate function according to
                          // datasheet (see above)
  };

  gpio_init_pin(&button_addr, &button_settings);

  // Get ADC channel

  AdcChannel adc_channel = NUM_ADC_CHANNELS;

  // Was told to not "worry about details", not sure what this does.
  adc_get_channel(adc_addr, &adc_channel);
  adc_set_channel(adc_channel, true);
  // adc_set_channel(ADC_CHANNEL_TEMP, true);

  LOG_DEBUG("ADC Channel: %d\n", adc_channel);

  // Taking documentation from gpio_it.h and interrupt_def.h, and making guesses as to settings
  InterruptSettings button_interrupt_settings = {
    INTERRUPT_TYPE_INTERRUPT,
    INTERRUPT_PRIORITY_NORMAL,
  };

  // Create a rising edge interrupt for the button to print data from the ADC (prv_get_adc_callback)
  gpio_it_register_interrupt(&button_addr, &button_interrupt_settings, INTERRUPT_EDGE_RISING,
                             prv_get_adc_callback, &adc_channel);

  while (true) {
    wait();
  }
}

static void prv_get_adc_callback(const GpioAddress *address, void *context) {
  AdcChannel *channel_data = (AdcChannel *)context;
  uint16_t data = 0;

  // Read scaled ADC value (use adc_read_raw to read raw ADC output)
  adc_read_converted(*channel_data, &data);
  // adc_read_raw(*channel_data, &data);
  LOG_DEBUG("ADC data: %d\n", data);
}
