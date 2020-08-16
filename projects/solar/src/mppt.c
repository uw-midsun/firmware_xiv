#include "mppt.h"

#include "gpio.h"
#include "mux.h"
#include "solar_boards.h"
#include "spv1020_mppt.h"

#define UNUSED_PIN \
  { GPIO_PORT_B, 1 }

#define MUX_ENABLE UNUSED_PIN  // we don't have an accessible enable pin but the driver needs one
#define MUX_OUTPUT UNUSED_PIN
#define SEL_PIN_0 \
  { .port = GPIO_PORT_B, .pin = 3 }
#define SEL_PIN_1 \
  { .port = GPIO_PORT_B, .pin = 4 }
#define SEL_PIN_2 \
  { .port = GPIO_PORT_B, .pin = 5 }

#define DISCONNECTED_MUX_OUTPUT 7

// will get the address and size of the demultiplexer
static MuxAddress s_mux_address = {
  .bit_width = 3,
  .sel_pins[0] = SEL_PIN_0,
  .sel_pins[1] = SEL_PIN_1,
  .sel_pins[2] = SEL_PIN_2,
  .mux_enable_pin = MUX_ENABLE,
  .mux_output_pin = MUX_OUTPUT,
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
