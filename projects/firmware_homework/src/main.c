// MidSun libraries
#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

// Interrupt callback function
static void prv_gpio_callback(const GpioAddress *address, void *context) {
  // Context should be the address of A6 as we want to read from this pin
  GpioAddress *adc_reading = context;
  uint16_t adc_data = 0;
  // Read the values from the ADC channel and output the reading
  // Check the data to see if it is valid first
  if (adc_read_converted_pin(*adc_reading, &adc_data) == STATUS_CODE_OK) {
    LOG_DEBUG("ADC Reading: \n", adc_data);
  } else {
    LOG_DEBUG("Invalid Reading: \n");
    // I'll just put invalid reading for anything else than STATUS CODE OK
  }
}

int main(void) {
  // Initializations
  interrupt_init();  // Enable interrupts
  gpio_init();       // Set up all GPIO pins
  gpio_it_init();    // Set GPIO pins for interrupts

  // Pin Defintions
  // GPIO pin (B2) that is connected to the button and "triggers" the interrupt
  GpioAddress button_pin = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };
  // Button pin settings
  GpioSettings button_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,  // Button is active high (off initially)
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,  // This pin doesn't need to do anything special
  };
  // GPIO pin (A6) that reads an ADC reading
  GpioAddress adc_pin = {
    .port = GPIO_PORT_A,
    .pin = 6,
  };
  // ADC pin settings
  GpioSettings adc_pin_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,  // Set the pin as an ADC channel
  };
  // Set up the pins
  gpio_init_pin(&button_pin, &button_pin_settings);
  gpio_init_pin(&adc_pin, &adc_pin_settings);

  // ADC Functions
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(adc_pin, true);  // Set up the ADC to read from pin A6

  // Interrupt
  // Default settings
  InterruptSettings settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,
    .priority = INTERRUPT_PRIORITY_NORMAL,
  };
  // Want to detect the falling edge for a button (when it is released)
  InterruptEdge edge = INTERRUPT_EDGE_FALLING;
  // Set Pin B2 to register interrupts and call the interrupt function when the button is pressed
  gpio_it_register_interrupt(&button_pin, &settings, edge, prv_gpio_callback, &adc_pin);

  // Infinite loop
  while (true) {
    wait();
  }
  return 0;
}
