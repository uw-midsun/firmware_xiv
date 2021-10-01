#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "wait.h"

// Pin A6 Reader Function
void read_pin_a6(const GpioAddress *address, void *context) {
  uint16_t pin_a6_data = 0;
  GpioAddress *pin_a6 = context;

  if (adc_read_converted_pin(*pin_a6, &pin_a6_data) == STATUS_CODE_OK) {
    LOG_DEBUG("Pin A6 Data: %d\n", pin_a6_data);
  } else {
    LOG_DEBUG("Failed to read Pin A6 Data");
  }
}

// Main
int main(void) {
  // Pin Initializations
  GpioAddress pin_a6 = { .port = GPIO_PORT_A, .pin = 6 };

  GpioSettings a6_settings = {
    GPIO_DIR_IN,
    GPIO_STATE_LOW,
    GPIO_RES_NONE,
    GPIO_ALTFN_ANALOG,
  };

  GpioAddress button_pin = { .port = GPIO_PORT_B, .pin = 2 };

  // Interrupt Initializations
  interrupt_init();

  InterruptSettings button_settings = { .type = INTERRUPT_TYPE_INTERRUPT,
                                        .priority = INTERRUPT_PRIORITY_NORMAL };

  gpio_init();
  gpio_it_init();
  gpio_init_pin(&pin_a6, &a6_settings);
  adc_init(ADC_MODE_SINGLE);

  // Channel Setup
  AdcChannel adc_channel = NUM_ADC_CHANNELS;
  adc_get_channel(pin_a6, &adc_channel);
  adc_set_channel(adc_channel, true);

  LOG_DEBUG("Pin A6 Channel: %d\n", adc_channel);

  // Interrupt Setup
  gpio_it_register_interrupt(&button_pin, &button_settings, INTERRUPT_EDGE_FALLING, read_pin_a6,
                             &pin_a6);

  while (true) {  // Actual Program Loop
    wait();
  }

  return 0;
}
