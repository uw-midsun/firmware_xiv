#include "mux.h"

StatusCode mux_init(MuxAddress *address) {
  // make sure the bit width isn't wonky
  if (address->bit_width > MAX_MUX_BIT_WIDTH) {
    return status_code(STATUS_CODE_INVALID_ARGS);
  }

  // initialize the select pins
  GpioSettings select_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  for (uint8_t i = 0; i < address->bit_width; i++) {
    status_ok_or_return(gpio_init_pin(&address->sel_pins[i], &select_settings));
  }

  return STATUS_CODE_OK;
}

StatusCode mux_set(MuxAddress *address, uint8_t selected) {
  if (selected >= (1 << address->bit_width)) {
    return status_code(STATUS_CODE_OUT_OF_RANGE);
  }

  for (uint8_t bit = 0; bit < address->bit_width; bit++) {
    GpioState select_state = ((selected & (1 << bit)) == 0) ? GPIO_STATE_LOW : GPIO_STATE_HIGH;
    status_ok_or_return(gpio_set_state(&address->sel_pins[bit], select_state));
  }
  return STATUS_CODE_OK;
}
