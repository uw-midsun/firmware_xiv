#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static GpioAddress adc_addr = {
  .port = GPIO_PORT_A,
  .pin = 6,
};

static GpioAddress button_addr = {
  .port = GPIO_PORT_B,
  .pin = 2,
};

static InterruptSettings s_interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

void read_from_adc(const GpioAddress *address, void *context) {
  uint16_t data = 0;
  if (adc_read_raw_pin(*(GpioAddress *)context, &data) == STATUS_CODE_OK) {
    LOG_DEBUG("pin A6 data: %d\n", data);
  } else {
    LOG_DEBUG("Failed to read data from pin A6");
  }
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  GpioSettings adc_read_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };

  gpio_init_pin(&adc_addr, &adc_read_settings);
  gpio_it_register_interrupt(&button_addr, &s_interrupt_settings, INTERRUPT_EDGE_FALLING,
                             read_from_adc, &adc_addr);

  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(adc_addr, true);

  while (true) {
  }
}
