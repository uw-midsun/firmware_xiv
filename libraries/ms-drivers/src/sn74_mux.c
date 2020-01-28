#include "sn74_mux.h"

StatusCode sn74_mux_init_mux(Sn74MuxAddress *address) {
  // initialize the select pins
  GpioSettings select_settings = {
    .direction = GPIO_DIR_OUT,
    .state = GPIO_STATE_LOW,
    .resistor = GPIO_RES_NONE,
    .alt_function = GPIO_ALTFN_NONE,
  };
  for (uint8_t i = 0; i < SN74_MUX_BIT_WIDTH; i++) {
    status_ok_or_return(gpio_init_pin(&address->sel_pins[i], &select_settings));
  }

  // initialize the mux output pin
  GpioSettings mux_output_settings = {
    .direction = GPIO_DIR_IN,  // what if we want to initialize it with GPIO_ALTFN_ANALOG?
    .alt_function = GPIO_ALTFN_NONE,
  };
  status_ok_or_return(gpio_init_pin(&address->mux_output_pin, &mux_output_settings));

  return STATUS_CODE_OK;
}

#include "log.h"

StatusCode sn74_mux_set(Sn74MuxAddress *address, uint8_t selected) {
  if (selected >= (1 << SN74_MUX_BIT_WIDTH)) {
    return STATUS_CODE_OUT_OF_RANGE;
  }

  for (uint8_t bit = 0; bit < SN74_MUX_BIT_WIDTH; bit++) {
    GpioState select_state = ((selected & (1 << bit)) == 0) ? GPIO_STATE_LOW : GPIO_STATE_HIGH;
    status_ok_or_return(gpio_set_state(&address->sel_pins[bit], select_state));
  }
  return STATUS_CODE_OK;
}
