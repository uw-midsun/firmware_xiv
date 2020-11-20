#include "gpio_set.h"

#include <string.h>

#include "babydriver_msg_defs.h"
#include "dispatcher.h"
#include "gpio.h"

static StatusCode prv_gpio_set_callback(uint8_t data[8], void *context, bool *tx_result) {
  GpioPort port = data[1];
  uint8_t pin = data[2];
  GpioState state = data[3];
  if (port >= NUM_GPIO_PORTS || pin >= GPIO_PINS_PER_PORT) {
    return STATUS_CODE_INVALID_ARGS;
  }
  // treat any nonzero state value as high
  if (state != GPIO_STATE_LOW) {
    state = GPIO_STATE_HIGH;
  }
  GpioAddress gpio_address = { .port = port, .pin = pin };
  GpioSettings gpio_settings = {
    .direction = GPIO_DIR_OUT,
    .state = state,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };

  status_ok_or_return(gpio_init_pin(&gpio_address, &gpio_settings));

  return STATUS_CODE_OK;
}

StatusCode gpio_set_init(void) {
  return dispatcher_register_callback(BABYDRIVER_MESSAGE_GPIO_SET, prv_gpio_set_callback, NULL);
}
