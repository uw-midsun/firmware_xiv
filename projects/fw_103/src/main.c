#include <stdbool.h>

#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static void prv_read_adc(const GpioAddress *btn_addr_ptr, void *context) {
  GpioAddress pot_addr = *(GpioAddress *)context;
  uint16_t pot_data = 0;
  adc_read_raw_pin(pot_addr, &pot_data);
  LOG_DEBUG("Potentiometer Data: %d\n", pot_data);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();
  gpio_it_init();

  // Potentiometer
  GpioAddress pot_addr = { .port = GPIO_PORT_A, .pin = 6 };
  GpioSettings pot_settings = { .direction = GPIO_DIR_IN,
                                .state = GPIO_STATE_LOW,
                                .resistor = GPIO_RES_NONE,
                                .alt_function = GPIO_ALTFN_ANALOG };

  gpio_init_pin(&pot_addr, &pot_settings);
  adc_init(ADC_MODE_SINGLE);
  adc_set_channel_pin(pot_addr, true);

  // Button
  GpioAddress btn_addr = { .port = GPIO_PORT_B, .pin = 2 };
  InterruptSettings btn_intrpt = { .type = INTERRUPT_TYPE_INTERRUPT,
                                   .priority = INTERRUPT_PRIORITY_NORMAL };

  gpio_it_register_interrupt(&btn_addr, &btn_intrpt, INTERRUPT_EDGE_FALLING, prv_read_adc,
                             &pot_addr);

  while (true) {
    gpio_it_trigger_interrupt(&btn_addr);
    delay_ms(1000);
  }
  return 0;
}
