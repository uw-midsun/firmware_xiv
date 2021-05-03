#include <stdint.h>
#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static AdcChannel reading_channel = NUM_ADC_CHANNELS;

static GpioAddress reading_addr = { .port = GPIO_PORT_A, .pin = 6 };

static GpioSettings reading_settings = {
  GPIO_DIR_IN,
  GPIO_STATE_LOW,
  GPIO_RES_NONE,
  GPIO_ALTFN_ANALOG,
};

static GpioAddress button_read_addr = { .port = GPIO_PORT_B, .pin = 2 };

static GpioSettings button_read_settings = {
  .direction = GPIO_DIR_IN,
  .state = GPIO_STATE_HIGH,
  .resistor = GPIO_RES_PULLDOWN,
  .alt_function = GPIO_ALTFN_NONE,
};

static InterruptSettings button_press = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static void prv_button_callback(const GpioAddress *address, void *context) {
  uint16_t reading = 0;
  adc_read_converted(reading_channel, &reading);
  LOG_DEBUG("reading in pin A6: %d\n", reading);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  gpio_init_pin(&reading_addr, &reading_settings);
  gpio_init_pin(&button_read_addr, &button_read_settings);

  adc_init(ADC_MODE_SINGLE);

  adc_get_channel(reading_addr, &reading_channel);
  adc_set_channel(reading_channel, true);
  adc_set_channel(ADC_CHANNEL_TEMP, true);

  gpio_it_register_interrupt(&button_read_addr, &button_press, INTERRUPT_EDGE_RISING,
                             prv_button_callback, &reading_addr);

  while (1) {
  }

  return 0;
}
