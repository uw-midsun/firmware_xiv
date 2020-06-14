#include "gpio.h"
#include "mux.h"
#include "spv1020_mppt.h"

// will get the address and size of the demultiplexer
static MuxAddress s_mux_address = {
  .bit_width = 3,
  .sel_pins[0] = { .port = GPIO_PORT_B, .pin = 3 },
  .sel_pins[1] = { .port = GPIO_PORT_B, .pin = 4 },
  .sel_pins[2] = { .port = GPIO_PORT_B, .pin = 5 },
  .mux_enable_pin = { .port = GPIO_PORT_A, .pin = 1 },  // need to check hardward for correct pin
};

StatusCode mppt_shut(SpiPort port, uint8_t pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  return spv1020_shut(port);
}

StatusCode mppt_turn_on(SpiPort port, uint8_t pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  return spv1020_turn_on(port);
}

StatusCode mppt_read_current(SpiPort port, uint16_t *current, uint8_t pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  return spv1020_read_current(port, current);
}

StatusCode mppt_read_voltage_in(SpiPort port, uint16_t *vin, uint8_t pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  return spv1020_read_voltage_in(port, vin);
}

StatusCode mppt_read_pwm(SpiPort port, uint16_t *pwm, uint8_t pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  return spv1020_read_pwm(port, pwm);
}

StatusCode mppt_read_status(SpiPort port, uint8_t *status, uint8_t pin) {
  status_ok_or_return(mux_set(&s_mux_address, pin));
  return spv1020_read_status(port, status);
}
