#include <stdint.h>
#include <stdlib.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static GpioAddress pot_address = {
  .port = GPIO_PORT_A,
  .pin = 6,
};

static GpioAddress button_address = {
  .port = GPIO_PORT_B,
  .pin = 2,
};

static GpioSettings pot_settings = {
  GPIO_DIR_IN,
  GPIO_STATE_LOW,
  GPIO_RES_NONE,
  GPIO_ALTFN_ANALOG,
};

static InterruptSettings interrupt_settings = {
  .type = INTERRUPT_TYPE_INTERRUPT,
  .priority = INTERRUPT_PRIORITY_NORMAL,
};

static void prv_button_interrupt_handler(const GpioAddress *address, void *context) {
  uint16_t pot_data = 0;
  if (adc_read_converted_pin(pot_address, &pot_data) == STATUS_CODE_OK) {
    LOG_DEBUG("potentiometer data: %d\n", pot_data);
  } else {
    LOG_DEBUG("Failed to read potentiomter data");
  }
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  gpio_init_pin(&pot_address, &pot_settings);

  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(pot_address, true);

  gpio_it_register_interrupt(&button_address, &interrupt_settings, INTERRUPT_EDGE_FALLING,
                             &prv_button_interrupt_handler, &pot_address);

  while (true) {
  }
}
