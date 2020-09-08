#include "mppt.h"

#include "gpio.h"
#include "mux.h"
#include "pin_defs.h"
#include "solar_boards.h"
#include "spv1020_mppt.h"

static MuxAddress s_mux_address = {
  .bit_width = 3,
  .sel_pins[0] = MUX_SEL_PIN_0,
  .sel_pins[1] = MUX_SEL_PIN_1,
  .sel_pins[2] = MUX_SEL_PIN_2,
  .mux_enable_pin = SOLAR_UNUSED_PIN,  // we don't have an accessible enable or output pin
  .mux_output_pin = SOLAR_UNUSED_PIN,  // but the mux driver needs them to be set
};

StatusCode mppt_init() {
  return mux_init(&s_mux_address);
}

StatusCode mppt_shut(SpiPort port, Mppt pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  StatusCode spv1020_code = spv1020_shut(port);
  status_ok_or_return(mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT));
  return spv1020_code;
}

StatusCode mppt_turn_on(SpiPort port, Mppt pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  StatusCode spv1020_code = spv1020_turn_on(port);
  status_ok_or_return(mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT));
  return spv1020_code;
}

StatusCode mppt_read_current(SpiPort port, uint16_t *current, Mppt pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  StatusCode spv1020_code = spv1020_read_current(port, current);
  status_ok_or_return(mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT));
  return spv1020_code;
}

StatusCode mppt_read_voltage_in(SpiPort port, uint16_t *vin, Mppt pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  StatusCode spv1020_code = spv1020_read_voltage_in(port, vin);
  status_ok_or_return(mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT));
  return spv1020_code;
}

StatusCode mppt_read_pwm(SpiPort port, uint16_t *pwm, Mppt pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  StatusCode spv1020_code = spv1020_read_pwm(port, pwm);
  status_ok_or_return(mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT));
  return spv1020_code;
}

StatusCode mppt_read_status(SpiPort port, uint8_t *status, Mppt pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  StatusCode spv1020_code = spv1020_read_status(port, status);
  status_ok_or_return(mux_set(&s_mux_address, DISCONNECTED_MUX_OUTPUT));
  return spv1020_code;
}
