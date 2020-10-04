// Created by Sudhish M on 2020-10-04.
// buttons_and_adc

#include "adc.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"

static GpioAddress adc_pin_address{ .port = GPIO_PORT_A, .pin = 6 };
static GpioAddress button_address{ .port = GPIO_PORT_B, .pin = 2 };

static GpioSettings button_settings = {
  .direction = GPIO_DIR_IN,
  .resistor = PIO_RES_PULLDOWN,
  // Reason for using pulldown: when no signal is received
  // from the button (unpressed), the value sent to the pin is 0
  .alt_function = GPIO_ALTFN_NONE
};

static GpioSettings adc_pin_settings = {
  .direction GPIO_DIR_IN, .resistor = GPIO_RES_NONE, .alt_function = GPIO_ALTFN_ANALOG
};

static InterruptSettings interrupt_settings =
    {
      .type = INTERRUPT_TYPE_INTERRUPT,
      .priority = INTERRUPT_PRIORITY_NORMAL,
    }

static void
prv_callback(const GpioAddress *address, void *context) {
  uint16_t reading = 0;
  GpioAddress *button_address = address;
  GpioAddress *adc_pin_address = context;
  //  enable the adc pin and log the values it reads
  adc_read_converted_pin(*adc_pin_address, &reading);
  LOG_DEBUG("ADC Reading: %i", reading);
}

int main() {
  gpio_init();
  interrupt_init();
  adc_init(ADC_MODE_SINGLE);

  gpio_init_pin(&adc_pin_address, &adc_pin_settings);
  gpio_init_pin(&button_address, &button_settings);

  adc_set_channel_pin(adc_pin_address, true);
  // I did not use step 4 mentioned in firmware_103 notes because
  // adc.h mentioned using the method illustrated in firmware_103 is deprecated.

  gpio_it_register_interrupt(&button_address, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             // callback is initiated when button's pin changes value to 0
                             &prv_callback, &adc_pin_address);

  while (true) {
  }

  return 0;
}
