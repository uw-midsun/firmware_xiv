#include "gpio_get.h"

#include "babydriver_msg_defs.h"
#include "can.h"
#include "can_msg_defs.h"
#include "can_transmit.h"
#include "dispatcher.h"
#include "gpio.h"
#include "status.h"

static StatusCode prv_callback_gpio_get(uint8_t data[8], void *context, bool *tx_result) {
  gpio_init();

  uint8_t port_number = data[1];
  uint8_t pin_number = data[2];

  if (port_number >= NUM_GPIO_PORTS || pin_number >= GPIO_PINS_PER_PORT) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  GpioAddress pin_address = { .port = port_number, .pin = pin_number };

  GpioSettings pin_settings = {
    .direction = GPIO_DIR_IN,         //
    .resistor = GPIO_RES_NONE,        //
    .alt_function = GPIO_ALTFN_NONE,  //
  };

  gpio_init_pin(&pin_address, &pin_settings);

  GpioState inputState = GPIO_STATE_LOW;
  gpio_get_state(&pin_address, &inputState);

  uint8_t state = 0;
  if (inputState == GPIO_STATE_HIGH) {
    state = 1;
  }

  CAN_TRANSMIT_BABYDRIVER(BABYDRIVER_MESSAGE_GPIO_GET_DATA, state, 0, 0, 0, 0, 0, 0);

  return STATUS_CODE_OK;
}

StatusCode gpio_get_init(void) {
  status_ok_or_return(dispatcher_register_callback(BABYDRIVER_MESSAGE_GPIO_GET_COMMAND,  //
                                                   prv_callback_gpio_get,                //
                                                   NULL));                               //
  return STATUS_CODE_OK;
}
