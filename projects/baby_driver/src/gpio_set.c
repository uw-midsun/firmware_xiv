#include "gpio_set.h"

#include <stddef.h>
#include <stdint.h>

#include "babydriver_msg_defs.h"
#include "dispatcher.h"
#include "gpio.h"
#include "status.h"

static StatusCode prv_callback(uint8_t data[8], void *context, bool *tx_result) {
  // lack of unpack is eww
  GpioAddress address = { .port = data[1], .pin = data[2] };
  GpioState state = data[3];
  GpioSettings settings = {
    .direction = GPIO_DIR_OUT,
    .state = state,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  return gpio_init_pin(&address, &settings);
}

StatusCode gpio_set_init(void) {
  return dispatcher_register_callback(BABYDRIVER_MESSAGE_GPIO_SET, prv_callback, NULL);
}
