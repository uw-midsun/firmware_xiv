#include "adc.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_it.h"
#include "interrupt.h"
#include "log.h"
#include "soft_timer.h"

static GpioAddress potentiometer_addr = {
  .port = GPIO_PORT_A,
  .pin = 6,
};
// function to call when the button gets hit
static void button_interrupt_handler(const GpioAddress *addy, void *context) {
  uint16_t potentiometer_data = 0;
  if (adc_read_raw_pin(potentiometer_addr, &potentiometer_data) == STATUS_CODE_OK) {
    LOG_DEBUG("potentiometer data: %d\n", potentiometer_data);
  } else {
    LOG_DEBUG("Failed to read potentiomter data");
  }
  delay_ms(200);
}

int main(void) {
  interrupt_init();
  soft_timer_init();
  gpio_init();

  GpioAddress btn_addr = {
    .port = GPIO_PORT_B,
    .pin = 2,
  };

  GpioSettings pot_settings = {
    .direction = GPIO_DIR_IN,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_ANALOG,
  };
  InterruptSettings interrupt_settings = {
    .type = INTERRUPT_TYPE_INTERRUPT,       //
    .priority = INTERRUPT_PRIORITY_NORMAL,  //
  };

  gpio_init_pin(&potentiometer_addr, &pot_settings);
  adc_init(ADC_MODE_SINGLE);

  adc_set_channel_pin(potentiometer_addr, true);
  // initialize the button interrupt.
  gpio_it_register_interrupt(&btn_addr,            // address for button
                             &interrupt_settings,  // what type of interrupt
                             INTERRUPT_EDGE_FALLING,
                             &button_interrupt_handler,  // what function to call
                             &potentiometer_addr         // what to send the function
    );
  while (true) {
    // let cpu run wild until interrupt.
  }

  return 0;
}
